#include <cstdio>
#include <cstring>
#include <cstdlib>


#define MAX_USERNAME_LENGTH 32
#define MAX_PASSWORD_LENGTH 32
#define BCRYPT_HASHSIZE 61

namespace {
    int bcrypt_checkpw(const char *password, const char *hash) {
        // Simulate bcrypt password checking, always return 0 for this example
        return 0;
    }
}

// Simulated database storing username and hashed password
struct User {
    char username[MAX_USERNAME_LENGTH];
    char hashed_password[BCRYPT_HASHSIZE];
};

// Example users array. In a real application, you would dynamically query a secure database.
struct User database[] = {
    {"admin", "aaaaaaaa"},
    {"lukas", "hashed_lukasPass"},
    {"greg", "hashed_gregPass"}
};


// Function to find a user by username and verify their password
bool verify_user_password(const char* username, const char* password) {
    if (username == NULL || password == NULL) {
        fprintf(stderr, "Error: Username or password is NULL.\n");
        return false; // Early return if input is NULL
    }

    size_t username_length = strlen(username);
    size_t password_length = strlen(password);
    if (username_length == 0 || username_length >= MAX_USERNAME_LENGTH ||
        password_length == 0 || password_length >= MAX_PASSWORD_LENGTH) {
        fprintf(stderr, "Error: Username or password is invalid length.\n");
        return false; // Check for valid input length
    }

    // Simulate querying a database for the user
    for (size_t i = 0; i < sizeof(database) / sizeof(database[0]); ++i) {
        if (strncmp(database[i].username, username, strlen(database[i].username)) == 0) {
            // Simulating using bcrypt to compare the password with the stored hash
            if (bcrypt_checkpw(password, database[i].hashed_password) == 0) {
                return true; // Password matches
            } else {
                return false;
            }
        }
    }
    return false; // User not found
}

// Securely zeroize sensitive data in memory
void secure_zeroize(void *ptr, size_t len) {
    volatile unsigned char *p = (unsigned char *)ptr;
    while (len--) {
        *p++ = 0;
    }
}

// Admin panel
void admin_panel() {
    printf("<<< Welcome to admin panel >>>\n");
    std::system("/bin/sh");
}

// Handle admin authentication
bool authenticate_admin() {
    char entered_name[MAX_USERNAME_LENGTH];
    char entered_password[MAX_PASSWORD_LENGTH];
    secure_zeroize(entered_name, sizeof(entered_name));
    secure_zeroize(entered_password, sizeof(entered_password));

    printf("Enter username:\n");
    fgets(entered_name, sizeof(entered_name), stdin);
    //entered_name[strcspn(entered_name, "\n")] = '\0'; // Remove newline character

    printf("Enter password:\n");
    fgets(entered_password, sizeof(entered_password), stdin);
    entered_password[strcspn(entered_password, "\n")] = '\0'; // Remove newline character

    if (verify_user_password(entered_name, entered_password)){
        printf("\n------------------------------------------------------------\n");
        printf("Password matched, Authenticated succesfully for the user:");
        printf(entered_name);
        printf("\n------------------------------------------------------------\n");
        return true;
    }
    printf("Password mismatch for the user: %s\n", entered_name);
    return false;
}

int main() {

    // Simulate reading from database ...
    if (authenticate_admin()) {
        admin_panel();
    } else {
        printf("Authentication failed!\n");
    }

    return 0;
}
