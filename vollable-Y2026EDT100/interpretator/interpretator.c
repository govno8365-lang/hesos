#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "interpretator.h"
#include "executor.h"

Environment env;

void interpret(ASTNode *root) {
    memset(&env, 0, sizeof(Environment));
    env.running = 1;

    /* Initialize environment with global scope */
    init_environment();

    /* Collect all labels before execution */
    collect_labels(root);

    /* Start execution from the program block */
    if (root && root->right && root->right->type == NODE_BLOCK) {
        execute_block(root->right);
    } else {
        throw_exception("Program has no main block");
    }

    /* Cleanup */
    cleanup_environment();
    clear_labels();

    if (env.running) {
        syscall_exit(0);
    }
}