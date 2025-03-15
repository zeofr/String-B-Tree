#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>

#define MAX_KEYS 3
#define MAX_CHILDREN (MAX_KEYS + 1)

typedef struct Node {
    int isLeaf;
    int keyCount;
    char *keys[MAX_KEYS];
    struct Node *children[MAX_CHILDREN];
} Node;

typedef struct {
    GtkWidget *drawing_area;
    Node *root;
    char current_prefix[256]; // Store current prefix for highlighting
} AppData;

Node *createNode(int isLeaf) {
    Node *node = (Node *)malloc(sizeof(Node));
    node->isLeaf = isLeaf;
    node->keyCount = 0;
    for (int i = 0; i < MAX_CHILDREN; i++) {
        node->children[i] = NULL;
    }
    return node;
}

void splitChild(Node *parent, int index) {
    Node *child = parent->children[index];
    Node *newChild = createNode(child->isLeaf);
    char *tempKeys[MAX_KEYS + 1]; // Temporary array for keys (3 keys + 1 for new key)
    Node *tempChildren[MAX_CHILDREN + 1] = {NULL}; // Temporary array for children

    // Copy keys and children from the child into temporary arrays
    for (int i = 0; i < child->keyCount; i++) {
        tempKeys[i] = child->keys[i];
    }
    if (!child->isLeaf) {
        for (int i = 0; i <= child->keyCount; i++) {
            tempChildren[i] = child->children[i];
        }
    }

    // Determine the middle index
    int mid = MAX_KEYS / 2; // For MAX_KEYS = 3, mid = 1 (2nd key)

    // Update the left child with keys before the middle key
    child->keyCount = mid;
    for (int i = 0; i < mid; i++) {
        child->keys[i] = tempKeys[i];
    }

    // Update the right child with keys after the middle key
    newChild->keyCount = MAX_KEYS - mid - 1; // For MAX_KEYS = 3, 3 - 1 - 1 = 1 key
    for (int i = 0; i < newChild->keyCount; i++) {
        newChild->keys[i] = tempKeys[mid + 1 + i];
    }

    // Update children for both nodes if not a leaf
    if (!child->isLeaf) {
        for (int i = 0; i <= mid; i++) {
            child->children[i] = tempChildren[i];
        }
        for (int i = 0; i <= newChild->keyCount; i++) {
            newChild->children[i] = tempChildren[mid + 1 + i];
        }
    }

    // Shift parent's children and keys to make space for the new child
    for (int i = parent->keyCount; i >= index + 1; i--) {
        parent->children[i + 1] = parent->children[i];
    }
    parent->children[index + 1] = newChild;

    for (int i = parent->keyCount - 1; i >= index; i--) {
        parent->keys[i + 1] = parent->keys[i];
    }

    // Promote the middle key to the parent
    parent->keys[index] = tempKeys[mid];
    parent->keyCount++;
}

void insertNonFull(Node *node, char *key) {
    int i = node->keyCount - 1;

    if (node->isLeaf) {
        // Insert the key into the correct position in the leaf node
        while (i >= 0 && strcmp(key, node->keys[i]) < 0) {
            node->keys[i + 1] = node->keys[i];
            i--;
        }
        node->keys[i + 1] = strdup(key);
        node->keyCount++;
    } else {
        // Find the child to descend into
        while (i >= 0 && strcmp(key, node->keys[i]) < 0) {
            i--;
        }
        i++;

        // Split the child if it is full
        if (node->children[i]->keyCount == MAX_KEYS) {
            splitChild(node, i);

            // Decide which child to descend into after the split
            if (strcmp(key, node->keys[i]) > 0) {
                i++;
            }
        }

        // Recur to insert the key into the appropriate child
        insertNonFull(node->children[i], key);
    }
}

void insert(Node **root, char *key) {
    Node *r = *root;
    if (r->keyCount == MAX_KEYS) {
        // Create a new root if the current root is full
        Node *newRoot = createNode(0);
        newRoot->children[0] = r;
        splitChild(newRoot, 0);
        *root = newRoot;

        // Decide which child to insert the key into
        int i = (strcmp(key, newRoot->keys[0]) > 0) ? 1 : 0;
        insertNonFull(newRoot->children[i], key);
    } else {
        insertNonFull(r, key);
    }
}





#define MAX_KEYS 3
#define MIN_KEYS (MAX_KEYS / 2)  // Minimum keys per node after deletion (rounded up)

int findKey(Node *node, char *key) {
    int idx = 0;
    while (idx < node->keyCount && strcmp(node->keys[idx], key) < 0) {
        idx++;
    }
    return idx;
}

void merge(Node *node, int idx) {
    Node *child = node->children[idx];
    Node *sibling = node->children[idx + 1];

    child->keys[child->keyCount] = strdup(node->keys[idx]);

    for (int i = 0; i < sibling->keyCount; i++) {
        child->keys[child->keyCount + 1 + i] = sibling->keys[i];
    }

    if (!child->isLeaf) {
        for (int i = 0; i <= sibling->keyCount; i++) {
            child->children[child->keyCount + 1 + i] = sibling->children[i];
        }
    }

    child->keyCount += sibling->keyCount + 1;

    // Shift keys and children in the parent node
    for (int i = idx + 1; i < node->keyCount; i++) {
        node->keys[i - 1] = node->keys[i];
        node->children[i] = node->children[i + 1];
    }

    node->keyCount--;  // Parent key count decreases

    free(sibling);  // Free the merged sibling node
}

void borrowFromLeft(Node *node, int idx) {
    Node *child = node->children[idx];
    Node *sibling = node->children[idx - 1];

    // Move the key from the parent to the child
    for (int i = child->keyCount - 1; i >= 0; i--) {
        child->keys[i + 1] = child->keys[i];
    }

    if (!child->isLeaf) {
        for (int i = child->keyCount; i >= 0; i--) {
            child->children[i + 1] = child->children[i];
        }
    }

    // Take the parent key and move it to the child's first position
    child->keys[0] = strdup(node->keys[idx - 1]);

    if (!child->isLeaf) {
        child->children[0] = sibling->children[sibling->keyCount];
    }

    // Move the sibling's last key up to the parent
    node->keys[idx - 1] = sibling->keys[sibling->keyCount - 1];

    sibling->keyCount--;
    child->keyCount++;
}

void borrowFromRight(Node *node, int idx) {
    Node *child = node->children[idx];
    Node *sibling = node->children[idx + 1];

    child->keys[child->keyCount] = strdup(node->keys[idx]);

    if (!child->isLeaf) {
        child->children[child->keyCount + 1] = sibling->children[0];
    }

    node->keys[idx] = sibling->keys[0];

    // Shift keys of sibling left by 1
    for (int i = 1; i < sibling->keyCount; i++) {
        sibling->keys[i - 1] = sibling->keys[i];
    }

    if (!sibling->isLeaf) {
        // Shift children of sibling left by 1
        for (int i = 1; i <= sibling->keyCount; i++) {
            sibling->children[i - 1] = sibling->children[i];
        }
    }

    sibling->keyCount--;
    child->keyCount++;
}

void deleteFromNode(Node *node, char *key) {
    int idx = findKey(node, key);

    if (idx < node->keyCount && strcmp(node->keys[idx], key) == 0) {
        // Key is found in this node
        if (node->isLeaf) {
            // If leaf node, remove the key directly
            for (int i = idx + 1; i < node->keyCount; i++) {
                node->keys[i - 1] = node->keys[i];
            }
            node->keyCount--;
        } else {
            // If not leaf node, need to handle internal node deletion
            if (node->children[idx]->keyCount > MIN_KEYS) {
                Node *child = node->children[idx];
                char *predecessor = child->keys[child->keyCount - 1];
                deleteFromNode(child, predecessor);
                node->keys[idx] = predecessor;
            } else if (node->children[idx + 1]->keyCount > MIN_KEYS) {
                Node *sibling = node->children[idx + 1];
                char *successor = sibling->keys[0];
                deleteFromNode(sibling, successor);
                node->keys[idx] = successor;
            } else {
                // Merge the nodes
                merge(node, idx);
                deleteFromNode(node->children[idx], key);
            }
        }
    } else if (!node->isLeaf) {
        // If not found in this node, recurse into the child
        Node *child = node->children[idx];

        if (child->keyCount <= MIN_KEYS) {
            if (idx > 0 && node->children[idx - 1]->keyCount > MIN_KEYS) {
                borrowFromLeft(node, idx);
            } else if (idx < node->keyCount && node->children[idx + 1]->keyCount > MIN_KEYS) {
                borrowFromRight(node, idx);
            } else {
                // Merge the node with a sibling
                if (idx < node->keyCount) {
                    merge(node, idx);
                } else {
                    merge(node, idx - 1);
                }
            }
        }
        deleteFromNode(child, key);  // Recurse into the child node
    }
}


void onDeleteClicked(GtkButton *button, gpointer data) {
    AppData *appData = (AppData *)data;
    GtkWidget *entry = g_object_get_data(G_OBJECT(button), "delete_entry");
    const char *text = gtk_entry_get_text(GTK_ENTRY(entry));

    if (strlen(text) > 0) {
        deleteFromNode(appData->root, (char *)text);
        gtk_widget_queue_draw(appData->drawing_area);
        gtk_entry_set_text(GTK_ENTRY(entry), "");
    }
}


void prefixSearch(Node *node, const char *prefix, char *result, int *count) {
    if (!node) return;

    for (int i = 0; i < node->keyCount; i++) {
        if (strncmp(node->keys[i], prefix, strlen(prefix)) == 0) {
            strcat(result, node->keys[i]);
            strcat(result, ", ");
            (*count)++;
        }
    }

    if (!node->isLeaf) {
        for (int i = 0; i <= node->keyCount; i++) {
            prefixSearch(node->children[i], prefix, result, count);
        }
    }
}

void drawNode(cairo_t *cr, Node *node, int x, int y, int depth, int width, const char *prefix) {
    if (!node) return;

    int nodeWidth = 300;  // Width for each node
    int nodeHeight = 100; // Height for each node
    int yOffset = 200;    // Vertical spacing between levels

    // Dynamically calculate horizontal spacing based on the number of child nodes at this depth
    int xOffset = width / (node->keyCount + 1);  // Adjust xOffset to avoid large gaps

    // Ensure the spacing doesn't get too small
    if (xOffset < nodeWidth) {
        xOffset = nodeWidth + 20;  // Set a minimum gap
    }

    // Draw keys in the current node
    for (int i = 0; i < node->keyCount; i++) {
        int keyX = x + (i - node->keyCount / 2.0) * (nodeWidth / node->keyCount);

        cairo_set_source_rgb(cr, 0.8, 0.9, 1.0); // Default node background color
        if (prefix && strncmp(node->keys[i], prefix, strlen(prefix)) == 0) {
            cairo_set_source_rgb(cr, 1.0, 0.5, 0.5); // Highlight matched prefix in red
        }
        cairo_rectangle(cr, keyX - nodeWidth / (2 * node->keyCount), y - nodeHeight / 2, nodeWidth / node->keyCount + 20, nodeHeight);
        cairo_fill_preserve(cr);
        cairo_set_source_rgb(cr, 0, 0, 0);
        cairo_stroke(cr);

        cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
        cairo_set_font_size(cr, 16);
        cairo_text_extents_t extents;
        cairo_text_extents(cr, node->keys[i], &extents);
        cairo_move_to(cr, keyX - extents.width / 2, y + extents.height / 2);
        cairo_show_text(cr, node->keys[i]);
    }

    // Draw child nodes and connect them
    if (!node->isLeaf) {
        for (int i = 0; i <= node->keyCount; i++) {
            int childX = x + (i - node->keyCount / 2.0) * xOffset;
            int childY = y + yOffset;

            // Draw connecting line
            cairo_set_source_rgb(cr, 0, 0, 0);
            cairo_move_to(cr, x, y + nodeHeight / 2);
            cairo_line_to(cr, childX, childY - nodeHeight / 2);
            cairo_stroke(cr);

            // Recursively draw the child node
            drawNode(cr, node->children[i], childX, childY, depth + 1, width / 2, prefix);
        }
    }
}



gboolean onDraw(GtkWidget *widget, cairo_t *cr, gpointer data) {
    AppData *appData = (AppData *)data;
    GtkAllocation allocation;
    gtk_widget_get_allocation(widget, &allocation);

    int width = allocation.width;
    int height = allocation.height;

    drawNode(cr, appData->root, width / 2, 50, 0, width, appData->current_prefix);
    return FALSE;
}

void onInsertClicked(GtkButton *button, gpointer data) {
    AppData *appData = (AppData *)data;
    GtkWidget *entry = g_object_get_data(G_OBJECT(button), "entry");
    const char *text = gtk_entry_get_text(GTK_ENTRY(entry));

    if (strlen(text) > 0) {
        insert(&appData->root, (char *)text);
        gtk_widget_queue_draw(appData->drawing_area);
        gtk_entry_set_text(GTK_ENTRY(entry), "");
    }
}

void onSearchClicked(GtkButton *button, gpointer data) {
    AppData *appData = (AppData *)data;
    GtkWidget *entry = g_object_get_data(G_OBJECT(button), "search_entry");
    const char *prefix = gtk_entry_get_text(GTK_ENTRY(entry));

    if (strlen(prefix) > 0) {
        strncpy(appData->current_prefix, prefix, sizeof(appData->current_prefix));
        appData->current_prefix[sizeof(appData->current_prefix) - 1] = '\0'; // Ensure null-termination
    } else {
        appData->current_prefix[0] = '\0'; // Clear prefix if empty
    }

    gtk_widget_queue_draw(appData->drawing_area);
}
int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    AppData appData = {0};
    appData.root = createNode(1);

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "B-Tree Visualization");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

    GtkWidget *entry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(hbox), entry, TRUE, TRUE, 0);

    GtkWidget *insertButton = gtk_button_new_with_label("Insert");
    gtk_box_pack_start(GTK_BOX(hbox), insertButton, FALSE, FALSE, 0);

    GtkWidget *searchEntry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(hbox), searchEntry, TRUE, TRUE, 0);

    GtkWidget *searchButton = gtk_button_new_with_label("Search Prefix");
    gtk_box_pack_start(GTK_BOX(hbox), searchButton, FALSE, FALSE, 0);

    GtkWidget *deleteEntry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(hbox), deleteEntry, TRUE, TRUE, 0);

    GtkWidget *deleteButton = gtk_button_new_with_label("Delete");
    gtk_box_pack_start(GTK_BOX(hbox), deleteButton, FALSE, FALSE, 0);

    // Create a scrolled window
    GtkWidget *scrolledWindow = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolledWindow), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_box_pack_start(GTK_BOX(vbox), scrolledWindow, TRUE, TRUE, 0);

    // Create the drawing area
    GtkWidget *drawingArea = gtk_drawing_area_new();
    gtk_widget_set_size_request(drawingArea, 1600, 1200); // Set a larger size for the drawing area
    gtk_container_add(GTK_CONTAINER(scrolledWindow), drawingArea);

    appData.drawing_area = drawingArea;

    g_object_set_data(G_OBJECT(insertButton), "entry", entry);
    g_signal_connect(insertButton, "clicked", G_CALLBACK(onInsertClicked), &appData);

    g_object_set_data(G_OBJECT(searchButton), "search_entry", searchEntry);
    g_signal_connect(searchButton, "clicked", G_CALLBACK(onSearchClicked), &appData);

    g_object_set_data(G_OBJECT(deleteButton), "delete_entry", deleteEntry);
    g_signal_connect(deleteButton, "clicked", G_CALLBACK(onDeleteClicked), &appData);

    g_signal_connect(drawingArea, "draw", G_CALLBACK(onDraw), &appData);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    gtk_widget_show_all(window);

    gtk_main();

    return 0;
}

