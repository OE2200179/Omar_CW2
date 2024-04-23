#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <sstream>

using namespace std;

const int MAX_USERS = 100;
const int MAX_CHATS = 100;
const int MAX_MESSAGES = 100;

struct User {
    string username;
    string password;
};
struct Message {
    string sender;
    string receiver;
    string content;
};
struct Chat {
    User* user1;
    User* user2;
    Message messages[MAX_MESSAGES];
    int messageCount = 0;
};
struct SecureChatApplication {
    User users[MAX_USERS];
    Chat chats[MAX_CHATS];
    int userCount = 0;
    int chatCount = 0;
};

const string userDataFile = "users.txt";
const string chatDataFile = "chats.txt";


void registerUser(SecureChatApplication& app, string username, string password);
User* authenticateUser(SecureChatApplication& app, string username, string password);
void displayLoginMenu();
void displayMainMenu();
void startChat(SecureChatApplication& app, User* user1, User* user2);
void sendMessage(Chat& chat, User* sender, User* receiver, string content, SecureChatApplication& app);
void printMessages(Chat& chat, User* user);
void viewChats(SecureChatApplication& app, User* user);
void deleteChat(SecureChatApplication& app, User* user1, User* user2);
User* searchUser(SecureChatApplication& app, string username);
void writeUserData(const SecureChatApplication& app);
void readUserData(SecureChatApplication& app);
void writeChatData(const SecureChatApplication& app);
void readChatData(SecureChatApplication& app);
string encryptMessage(const string& message, int shift);
string decryptMessage(const string& encryptedMessage, int shift);



string encryptMessage(const string& message, int shift) {
    string encryptedMessage = message;
    for (char& c : encryptedMessage) {
        if (isalpha(c)) {
            char base = isupper(c) ? 'A' : 'a';
            c = ((c - base + shift) % 26) + base;
        }
    }
    return encryptedMessage;
}

string decryptMessage(const string& encryptedMessage, int shift) {
    return encryptMessage(encryptedMessage, 26 - shift);
}
void writeUserData(const SecureChatApplication& app) {
    ofstream outFile(userDataFile);
    for (int i = 0; i < app.userCount; ++i) {
        outFile << app.users[i].username << " " << app.users[i].password << endl;
    }
    outFile.close();
}
void readUserData(SecureChatApplication& app) {
    ifstream inFile(userDataFile);
    if (inFile.is_open()) {
        string username, password;
        while (inFile >> username >> password && app.userCount < MAX_USERS) {
            User newUser = {username, password};
            app.users[app.userCount++] = newUser;
        }
        inFile.close();
    } else {
        cout << "Error: Unable to open user data file." << endl;
    }
}
void writeChatData(const SecureChatApplication& app) {
    ofstream outFile(chatDataFile);
    if (outFile.is_open()) {
        for (int i = 0; i < app.chatCount; ++i) {
            outFile << app.chats[i].user1->username << " " << app.chats[i].user2->username << endl;
            for (int j = 0; j < app.chats[i].messageCount; ++j) {

                string encryptedMessage = encryptMessage(app.chats[i].messages[j].content, 3);
                  outFile << app.chats[i].messages[j].sender << " " << app.chats[i].messages[j].receiver << " " << encryptedMessage << endl;
            }
            outFile << endl;
        }
        outFile.close();
    } else {
        cout << "Error: Unable to open chat data file for writing." << endl;
    }
}
void readChatData(SecureChatApplication& app) {
    ifstream inFile(chatDataFile);
    if (inFile.is_open()) {
        string sender, receiver, content;
        while (inFile >> sender >> receiver) {
            getline(inFile, content);
            if (!content.empty()) {
                User* user1 = searchUser(app, sender);
                User* user2 = searchUser(app, receiver);
                if (user1 != nullptr && user2 != nullptr) {
                    bool chatFound = false;
                    for (int i = 0; i < app.chatCount; ++i) {
                        if ((app.chats[i].user1 == user1 && app.chats[i].user2 == user2) || (app.chats[i].user1 == user2 && app.chats[i].user2 == user1)) {
                            chatFound = true;
                             string decryptedMessage = decryptMessage(content, 3);
                              Message newMessage = {sender, receiver, decryptedMessage};
                            app.chats[i].messages[app.chats[i].messageCount++] = newMessage;
                            break;
                        }
                    }
                    if (!chatFound) {
                        Chat newChat = {user1, user2};
                         string decryptedMessage = decryptMessage(content, 3);
                          Message newMessage = {sender, receiver, decryptedMessage};

                        newChat.messages[newChat.messageCount++] = newMessage;
                        app.chats[app.chatCount++] = newChat;
                    }
                } else {
                    cout << "Error: Invalid user(s) in chat data." << endl;
                }
            }
        }
        inFile.close();
    } else {
        cout << "Error: Unable to open chat data file." << endl;
    }
}
void registerUser(SecureChatApplication& app, string username, string password) {
    for (int i = 0; i < app.userCount; ++i) {
        if (app.users[i].username == username) {
            cout << "Error: Username already exists." << endl;
            return;
        }
    }
    User newUser = {username, password};
    app.users[app.userCount++] = newUser;
    cout << "User registered successfully." << endl << endl;
    writeUserData(app);
}
User* authenticateUser(SecureChatApplication& app, string username, string password) {
    for (int i = 0; i < app.userCount; ++i) {
        if (app.users[i].username == username && app.users[i].password == password) {
            return &app.users[i];
        }
    }
    return nullptr;
}
void sendMessage(Chat& chat, User* sender, User* receiver, string content, SecureChatApplication& app) {

    bool chatExists = false;
    Chat* existingChat = nullptr;
    for (int i = 0; i < app.chatCount; ++i) {
        if ((app.chats[i].user1 == sender && app.chats[i].user2 == receiver) || (app.chats[i].user1 == receiver && app.chats[i].user2 == sender)) {
            chatExists = true;
            existingChat = &app.chats[i];
            break;
            }
    }
    if (!chatExists) {
        if (app.chatCount >= MAX_CHATS) {
            cout << "Error: Maximum chat limit reached. Cannot create a new chat." << endl;
            return;
        }
        app.chats[app.chatCount++] = {sender, receiver};
        existingChat = &app.chats[app.chatCount - 1];
    }
    string encryptedMessage = encryptMessage(content, 3);
    Message newMessage = {sender->username, receiver->username, encryptedMessage};
    existingChat->messages[existingChat->messageCount++] = newMessage;
    cout << "Message sent successfully." << endl << endl;
    writeChatData(app);
}
void deleteChat(SecureChatApplication& app, User* user1, User* user2) {
    for (int i = 0; i < app.chatCount; ++i) {
        if ((app.chats[i].user1 == user1 && app.chats[i].user2 == user2) || (app.chats[i].user1 == user2 && app.chats[i].user2 == user1)) {
            for (int j = i; j < app.chatCount - 1; ++j) {
                app.chats[j] = app.chats[j + 1];
            }
            app.chatCount--;
            cout << "Chat deleted successfully." << endl << endl;
            writeChatData(app);
            return;
        }
    }
    cout << "Chat not found." << endl;
}
User* searchUser(SecureChatApplication& app, string username) {
    for (int i = 0; i < app.userCount; ++i) {
        if (app.users[i].username == username) {
            return &app.users[i];
        }
    }
    return nullptr;
}
void displayLoginMenu() {
    cout << "Welcome to Secure Chat Application : " << endl;
    cout << "---------------------------------" << endl;
    cout << "1. Log in" << endl;
    cout << "2. Register" << endl;
    cout << "3. Exit" << endl;
    cout << "Enter your choice: ";
}
void viewChats(SecureChatApplication& app, User* user) {
    cout << "Chats for user: " << user->username << endl << endl;
    for (int i = 0; i < app.chatCount; ++i) {
        if (app.chats[i].user1 == user || app.chats[i].user2 == user) {
            cout << "Chat with " << (app.chats[i].user1 == user ? app.chats[i].user2->username : app.chats[i].user1->username) << endl;
            printMessages(app.chats[i], user);
        }
    }
    cout << endl;
}
void printMessages(Chat& chat, User* user) {
    for (int i = 0; i < chat.messageCount; ++i) {
        if (chat.messages[i].sender == user->username || chat.messages[i].receiver == user->username) {
            string decryptedMessage = decryptMessage(chat.messages[i].content, 3);
            cout << chat.messages[i].sender << " -> " << chat.messages[i].receiver << ": " << decryptedMessage << endl;
        }
    }
    cout << "---------------------------------" << endl;
}
void displayMainMenu() {
    cout << "Welcome: " << endl;
    cout << "1. Start the chat and send a message" << endl;
    cout << "2. View available chats" << endl;
    cout << "3. Delete a chat" << endl;
    cout << "4. Log out" << endl;
    cout << "---------------------------------" << endl;
    cout << "Enter your choice: ";
}

int main() {
    SecureChatApplication app;
    readUserData(app);
    readChatData(app);

    int loginChoice;
    do {
        displayLoginMenu();
        cin >> loginChoice;
        if (cin.fail()) {
            cout << "Invalid input. Please enter a valid choice." << endl;
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            continue;
        }

        switch (loginChoice) {

            case 1: {
                string username, password;
                cout << "Enter username: ";
                cin >> username;
                cout << "Enter password: ";
                cin >> password;
                User* currentUser = authenticateUser(app, username, password);
                if (currentUser != nullptr) {
                    int mainChoice;
                    do {
                        displayMainMenu();
                        cin >> mainChoice;
                        switch (mainChoice) {

                            case 1: {
                                string receiver, message;
                                cout << "Enter the username of the receiver you want to chat with: ";
                                cin >> receiver;
                                User* receiverUser = searchUser(app, receiver);
                                if (receiverUser != nullptr) {
                                    cout << "Enter the message: ";
                                    cin.ignore();
                                    getline(cin, message);

                                    sendMessage(app.chats[0], currentUser, receiverUser, message, app);
                                } else {
                                    cout << "Receiver not found." << endl;
                                }
                                break;
                            }

                            case 2: {
                                viewChats(app, currentUser);
                                break;
                            }

                            case 3: {
                                string otherUsername;
                                cout << "Enter the username of the user you want to delete the chat with: ";
                                cin >> otherUsername;
                                User* otherUser = searchUser(app, otherUsername);
                                if (otherUser != nullptr) {
                                    deleteChat(app, currentUser, otherUser);
                                } else {
                                    cout << "User not found." << endl;
                                }
                                break;
                            }

                            case 4: {
                                cout << "Logging out..." << endl;
                                break;
                            }
                            default:
                                cout << "Invalid choice. Please try again." << endl;
                        }
                    } while (mainChoice != 4);
                } else {
                    cout << "Invalid username or password." << endl;
                }
                break;
            }

            case 2: {
                string username, password;
                cout << "Enter username: ";
                cin >> username;
                cout << "Enter password: ";
                cin >> password;
                registerUser(app, username, password);
                break;
            }

            case 3: {
                cout << "Exiting application." << endl;
                break;
            }
            default:
                cout << "Invalid choice. Please try again." << endl;
        }
    } while (loginChoice != 3);

    return 0;
}
