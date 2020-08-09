//
// Created by Ujjwal Krishnamurthi on 7/28/20.
//

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include "btree.c"

// DB Access methods
Pager* pager_open(const char* filename) {
    // Using bitwise operators with signed integers, but okay for now since macros are bit flags
    int fd = open(filename, O_RDWR | O_CREAT, S_IWUSR | S_IRUSR);
    if(fd == -1) {
        printf("Unable to open file.\n");
        exit(EXIT_FAILURE);
    }

    off_t file_length = lseek(fd, 0, SEEK_END);
    Pager* pager = malloc(sizeof(Pager));
    pager->file_descriptor = fd;
    pager->file_length = file_length;
    pager->num_pages = (file_length / PAGE_SIZE);

    if(file_length % PAGE_SIZE != 0) {
        printf("Db file is not a whole number of pages. Corrupt file.\n");
        exit(EXIT_FAILURE);
    }

    for(uint32_t i = 0; i < TABLE_MAX_PAGES; i++) {
        pager->pages[i] = NULL;
    }

    return pager;
}

void* get_page(Pager* pager, uint32_t page_num) {
    if(page_num > TABLE_MAX_PAGES) {
        printf("Tried to fetch page number out of bounds. %d > %d\n", page_num, TABLE_MAX_PAGES);
        exit(EXIT_FAILURE);
    }

    if(pager->pages[page_num] == NULL) {
        // Cache miss, need to dynamically allocate memory to generate new page and load from file
        void* page = malloc(PAGE_SIZE);
        uint32_t num_pages = pager->file_length / PAGE_SIZE;
        // Saving partial page at the end of the file
        if (pager->file_length % PAGE_SIZE) {
            num_pages += 1;
        }

        if(page_num <= num_pages) {
            lseek(pager->file_descriptor, page_num * PAGE_SIZE, SEEK_SET);
            ssize_t bytes_read = read(pager->file_descriptor, page, PAGE_SIZE);
            if(bytes_read == -1) {
                printf("Error reading file %d\n", errno);
                exit(EXIT_FAILURE);
            }
        }
        pager->pages[page_num] = page;

        if(page_num >= pager->num_pages) {
            pager->num_pages = page_num + 1;
        }
    }
    return pager->pages[page_num];
}

void pager_flush(Pager* pager, uint32_t page_num) {
    if(pager->pages[page_num] == NULL) {
        printf("Tried to flush null page.\n");
        exit(EXIT_FAILURE);
    }

    off_t offset = lseek(pager->file_descriptor, page_num * PAGE_SIZE, SEEK_SET);
    if(offset == -1) {
        printf("Error seeking: %d\n", errno);
        exit(EXIT_FAILURE);
    }

    ssize_t bytes_written = write(pager->file_descriptor, pager->pages[page_num], PAGE_SIZE);
    if(bytes_written == -1) {
        printf("Error writing: %d\n", errno);
        exit(EXIT_FAILURE);
    }
}

Table* db_open(const char* filename) {
    Pager* pager = pager_open(filename);
    Table* table = malloc(sizeof(Table));
    table->pager = pager;
    table->root_page_num = 0;

    if(pager->num_pages == 0) {
        // New DB file. Initialize page 0 as leaf node
        void* root_node = get_page(pager, 0);
        initialize_leaf_node(root_node);
        set_node_root(root_node, true);
    }
    return table;
}

void db_close(Table* table) {
    Pager* pager = table->pager;
    //uint32_t num_full_pages = table->num_rows / ROWS_PER_PAGE;

    for(uint32_t i = 0; i < pager->num_pages; i++) {
        if(pager->pages[i] == NULL) continue;
        pager_flush(pager, i);
        free(pager->pages[i]);
        pager->pages[i] = NULL;
    }

    int result = close(pager->file_descriptor);
    if(result == -1) {
        printf("Error closing DB file.\n");
        exit(EXIT_FAILURE);
    }

    for(uint32_t i = 0; i < TABLE_MAX_PAGES; i++) {
        void* page = pager->pages[i];
        if(page) {
            free(page);
            pager->pages[i] = NULL;
        }
    }
    free(pager);
    free(table);
}

Cursor* table_start(Table* table) {
    Cursor* cursor = malloc(sizeof(Cursor));
    cursor->table = table;
    cursor->page_num = table->root_page_num;
    cursor->cell_num = 0;

    void* root_node = get_page(table->pager, table->root_page_num);
    uint32_t num_cells = *leaf_node_num_cells(root_node);
    cursor->end_of_table = (num_cells == 0);
    return cursor;
}

void* cursor_value(Cursor* cursor) {
    uint32_t page_num = cursor->page_num;
    void* page = get_page(cursor->table->pager, page_num);
    return leaf_node_value(page, cursor->cell_num);
}

void cursor_advance(Cursor* cursor) {
    uint32_t page_num = cursor->page_num;
    void* node = get_page(cursor->table->pager, page_num);

    cursor->cell_num += 1;
    if(cursor->cell_num >= *leaf_node_num_cells(node)) {
        cursor->end_of_table = true;
    }
}

// DB modification methods
Cursor* leaf_node_find(Table* table, uint32_t page_num, uint32_t key) {
    void* node = get_page(table->pager, page_num);
    uint32_t num_cells = *leaf_node_num_cells(node);

    Cursor* cursor = malloc(sizeof(Cursor));
    cursor->table = table;
    cursor->page_num = page_num;

    // Binary search
    uint32_t min_index = 0;
    uint32_t max_index_next = num_cells;
    while(max_index_next != min_index) {
        uint32_t index = (min_index + max_index_next) / 2;
        uint32_t key_at_index = *leaf_node_key(node, index);
        if(key == key_at_index) {
            cursor->cell_num = index;
            return cursor;
        }
        if (key < key_at_index) {
            max_index_next = index;
        } else {
            min_index = index + 1;
        }
    }
    cursor->cell_num = min_index;
    return cursor;
}

Cursor* internal_node_find(Table* table, uint32_t page_num, uint32_t key) {
    void* node = get_page(table->pager, page_num);
    uint32_t num_keys = *internal_node_num_keys(node);

    // Binary search
    uint32_t min_index = 0;
    uint32_t max_index_next = num_keys; // One more child than key
    while(min_index != max_index_next) {
        uint32_t index = (min_index + max_index_next) / 2;
        uint32_t key_to_right = *internal_node_key(node, index);
        if(key_to_right >= key) {
            max_index_next = index;
        } else {
            min_index = index + 1;
        }
    }
    uint32_t child_num = *internal_node_child(node, min_index);
    void* child = get_page(table->pager, child_num);
    switch (get_node_type(child)) {
        case NODE_LEAF:
            return leaf_node_find(table, child_num, key);
        case NODE_INTERNAL:
            return internal_node_find(table, child_num, key);
    }
}


/**
 * Return the position of the given key in the table
 * If key not present, return index where should be inserted
 * */
Cursor* table_find(Table* table, uint32_t key) {
    uint32_t root_page_num = table->root_page_num;
    void* root_node = get_page(table->pager, root_page_num);

    if(get_node_type(root_node) == NODE_LEAF) {
        return leaf_node_find(table, root_page_num, key);
    } else {
        return internal_node_find(table, root_page_num, key);
    }
}

void create_new_root(Table* table, uint32_t right_child_page_num) {
    /**
     * Handle splitting the root node
     * Root becomes left child and right child address passed in
     * Re-initialize root page to have new root node with aforementioned two children
     */
     void* root = get_page(table->pager, table->root_page_num);
     void* right_child = get_page(table->pager, right_child_page_num);
     uint32_t left_child_page_num = get_unused_page_num(table->pager);
     void* left_child = get_page(table->pager, left_child_page_num);

     /* Copy old root data to left child*/
     memcpy(left_child, root, PAGE_SIZE);
     set_node_root(left_child, false);

     /* Initialize root node as internal node with one key and two children*/
     initialize_internal_node(root);
     set_node_root(root, true);
     *internal_node_num_keys(root) = 1;
     *internal_node_child(root, 0) = left_child_page_num;
     uint32_t left_child_max_key = get_node_max_key(left_child);
     *internal_node_key(root, 0) = left_child_max_key;
     *internal_node_right_child(root) = right_child_page_num;
}

/**
 * Create new node and move over half of the cells
 * Insert value in one of the cells
 * Update parent or create new parent
*/
void leaf_node_split_and_insert(Cursor* cursor, uint32_t key, Row* value) {
    void* old_node = get_page(cursor->table->pager, cursor->page_num);
    uint32_t new_page_num = get_unused_page_num(cursor->table->pager);
    void* new_node = get_page(cursor->table->pager, new_page_num);
    initialize_leaf_node(new_node);

    for(int32_t i = LEAF_NODE_MAX_CELLS; i >= 0; i--) {
        void* destination_node = i >= LEAF_NODE_LEFT_SPLIT_COUNT ? new_node : old_node;
        uint32_t index_in_node = i % LEAF_NODE_LEFT_SPLIT_COUNT;
        void* destination = leaf_node_cell(destination_node, index_in_node);

        if(i == cursor->cell_num) serialize_row(value, destination);
        else if(i > cursor->cell_num) memcpy(destination, leaf_node_cell(old_node, i - 1), LEAF_NODE_CELL_SIZE);
        else memcpy(destination, leaf_node_cell(old_node, i), LEAF_NODE_CELL_SIZE);
    }

    /* Update cell counts on both leaf nodes*/
    *(leaf_node_num_cells(old_node)) = LEAF_NODE_LEFT_SPLIT_COUNT;
    *(leaf_node_num_cells(new_node)) = LEAF_NODE_RIGHT_SPLIT_COUNT;

    if(is_node_root(old_node)) {
        return create_new_root(cursor->table, new_page_num);
    } else {
        printf("Need to implement updating parent after split\n");
        exit(EXIT_FAILURE);
    }
}

void leaf_node_insert(Cursor* cursor, uint32_t key, Row* value) {
    void* node = get_page(cursor->table->pager, cursor->page_num);
    uint32_t num_cells = *leaf_node_num_cells(node);

    if(num_cells >= LEAF_NODE_MAX_CELLS) {
        leaf_node_split_and_insert(cursor, key, value);
        return;
    }
    if(cursor->cell_num < num_cells) {
        // Make room for new cell
        for(uint32_t i = num_cells; i > cursor->cell_num; i--) {
            // Fix buffer overflow issues – method only checks if overflows on next insert, should check every insert
            memcpy(leaf_node_cell(node, i), leaf_node_cell(node, i - 1), LEAF_NODE_CELL_SIZE);
        }
    }
    *(leaf_node_num_cells(node)) += 1;
    *(leaf_node_key(node, cursor->cell_num)) = key;
    serialize_row(value, leaf_node_value(node, cursor->cell_num));
}
