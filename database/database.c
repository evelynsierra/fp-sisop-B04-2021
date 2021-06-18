//PERINTAH COMPILE :
//gcc -w  database.c -lpthread -o database

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <wait.h>
#include <ctype.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <dirent.h>
#include <signal.h>
#include <setjmp.h>

void *client_provider(void *);

#define str_size 1024
#define SIZE_BUF 100

struct user_model
{
    char username[128];
    char password[128];
    char databases[100][256];
};

static jmp_buf context;
int dbIndex = 0, userIndex = 0;
char active_database[str_size], active_user[str_size];
char database_query[str_size];
char databases[100][256];
struct user_model user_list[100];
char cwd[str_size];

static void signal_listener(int signo)
{
    longjmp(context, 1);
}

static void catch_segmentation_fault(int catch)
{
    struct sigaction sa;

    if (catch)
    {
        memset(&sa, 0, sizeof(struct sigaction));
        sa.sa_handler = signal_listener;
        sa.sa_flags = SA_RESTART;
        sigaction(SIGSEGV, &sa, NULL);
    }
    else
    {
        sigemptyset(&sa);
        sigaddset(&sa, SIGSEGV);
        sigprocmask(SIG_UNBLOCK, &sa, NULL);
    }
}

static int run_with_segementation_fault_checking(int (*func)())
{
    catch_segmentation_fault(1);

    if (setjmp(context))
    {
        catch_segmentation_fault(0);
        return 0;
    }

    int return_status = func();
    catch_segmentation_fault(0);
    return return_status;
}

void create_file(char *filename)
{
    if (access(filename, F_OK))
    {
        FILE *fp = fopen(filename, "w+");
        fclose(fp);
    }
}

char *uppercase(char *str)
{
    if (str != NULL)
    {
        char *start = str;
        while (*str)
        {
            *str = toupper(*str);
            str++;
        }
        return start;
    }
    else
    {
        return NULL;
    }
}

char *lowercase(char *str)
{
    if (str != NULL)
    {
        char *start = str;
        while (*str)
        {
            *str = tolower(*str);
            str++;
        }
        return start;
    }
    else
    {
        return NULL;
    }
}

char *remove_foward_and_trailing_whitespace(char *str)
{
    if (str != NULL)
    {
        char *end;

        while (isspace((unsigned char)*str))
            str++;

        if (*str == 0)
            return str;

        end = str + strlen(str) - 1;
        while (end > str && isspace((unsigned char)*end))
            end--;

        end[1] = '\0';

        return str;
    }
    else
    {
        return NULL;
    }
}

char *get_string_between(char *str, char *leftStr, char *rightStr)
{
    if (leftStr == NULL)
    {
        char tmp[str_size] = {0};
        sprintf(tmp, "LEFTBORDER%s", str);

        return get_string_between(tmp, "LEFTBORDER", rightStr);
    }
    else if (rightStr == NULL)
    {
        char tmp[str_size] = {0};
        sprintf(tmp, "%sRIGHTBORDER", str);
        return get_string_between(tmp, leftStr, "RIGHTBORDER");
    }

    char *res = NULL, *start, *end;

    start = strstr(str, leftStr);
    if (start)
    {
        start += strlen(leftStr);
        end = strstr(start, rightStr);
        if (end)
        {
            res = (char *)malloc(end - start + 1);
            memcpy(res, start, end - start);
            res[end - start] = '\0';
        }
    }
    if (res == NULL)
    {
        return NULL;
    }
    else
    {
        return res;
    }
}

int check_database(char *databaseName)
{
    int i = dbIndex;
    while (i--)
    {
        if (!strcmp(databases[i], databaseName))
        {
            return 1;
        };
    }
    return 0;
}

void append_file(char *filename, char *string)
{
    FILE *fp = fopen(filename, "a+");
    printf("Append {%s} to {%s}\n", string, filename);
    fprintf(fp, "%s\n", string);
    fclose(fp);
}

int run_db_command()
{
    char *query = remove_foward_and_trailing_whitespace(database_query);
    char *cmd = uppercase(get_string_between(query, NULL, " "));
    printf("Command Input: [%s]\n", cmd);

    /*
        QUERY CREATE
    */
    if (!strcmp(cmd, "CREATE"))
    {
        char *subcmd = uppercase(get_string_between(query, "CREATE ", " "));
        printf("\t[%s]\n", subcmd);

        /*
            CREATE USER
        */
        if (!strcmp(subcmd, "USER"))
        {
            char *username, *password;
            char tmp[str_size];

            username = get_string_between(query, "USER ", " ");
            printf("\t   --  Username: {%s}\n", username);
            sprintf(tmp, "%s ", username);

            free(subcmd);
            subcmd = uppercase(get_string_between(query, tmp, " "));
            printf("\t   --  [%s]\n", subcmd);

            if (!strcmp(subcmd, "IDENTIFIED"))
            {
                free(subcmd);
                subcmd = uppercase(get_string_between(query, "IDENTIFIED ", " "));
                printf("\t   --  [%s]\n", subcmd);

                if (!strcmp(subcmd, "BY"))
                {
                    char tmpstr[1024];
                    password = get_string_between(query, "BY ", ";");
                    printf("\t   --  Password: {%s}\n", password);

                    sprintf(tmpstr, "%s:::%s:::\n", username, password);
                    append_file("users", tmpstr);
                    return 1;
                }
                else
                {
                    printf("[BY] NOT FOUND\n");
                    return 0;
                }
            }
            else
            {
                printf("[IDENTIFIED] NOT FOUND\n");
                return 0;
            }
        }
        /* 
            QUERY CREATE DATABASE
        */
        else if (!strcmp(subcmd, "DATABASE"))
        {
            char *databaseName = lowercase(get_string_between(query, "DATABASE ", ";"));
            char filepath[str_size];
            sprintf(filepath, "%s/%s", cwd, databaseName);
            printf("\t   --  Table Name: {%s}\n", databaseName);
            printf("\t   --  Filepath : {%s}\n", filepath);

            if (!check_database(databaseName))
            {
                printf("\t   Making Database: %s\n", databaseName);
                mkdir(filepath, 777);
                append_file("databases", databaseName);
                return 1;
            }
            else
            {
                printf("\t   DATABASE EXISTS!", databaseName);
                return 0;
            }
        }
        /*
            QUERY CREATE TABLE
        */
        else if (!strcmp(subcmd, "TABLE"))
        {
            char tmp[str_size], filepath[str_size], header[str_size];
            char *tableName = lowercase(get_string_between(query, "TABLE ", " "));
            printf("\t   --  Table Name: {%s}\n", tableName);

            sprintf(tmp, "%s (", tableName);
            char *columns = lowercase(get_string_between(query, tmp, ");"));

            char *p = NULL;
            char col[100], *colname, *type, *tmpsave;

            sprintf(filepath, "%s/%s/%s", cwd, active_database, tableName);
            memset(header, 0, sizeof(header));
            if (access(filepath, F_OK))
            {
                printf("\t   --  Filepath: %s\n");
                printf("\t      --  Columns: \n");
                create_file(filepath);

                int colIndex = 0;
                while (1)
                {
                    tmpsave = remove_foward_and_trailing_whitespace(strtok_r(NULL, ",", &p));
                    if (tmpsave != NULL)
                    {
                        printf("\t         --  %s\n", tmpsave);
                        strcpy(col[colIndex], tmpsave);
                    }
                    else
                        break;

                    colIndex++;
                }

                return 1;
            }
            else
            {
                printf("\t      TABLE EXISTS!\n");
                return 0;
            }
        }

        free(subcmd);
        return 0;
    }
    return 0;
}

int main(int argc, char *argv[])
{
    //Checking apakah file ada
    if (access("databases", F_OK))
    {
        FILE *fp = fopen("databases", "w+");
        fclose(fp);
    }
    if (access("users", F_OK))
    {
        FILE *fp = fopen("users", "w+");
        fclose(fp);
    }

    getcwd(cwd, str_size);
    int socket_file_descriptor, client_sock, c, *new_sock;
    struct sockaddr_in server, client;

    //Create socket
    socket_file_descriptor = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_file_descriptor == -1)
    {
        printf("Could not create socket");
    }
    puts("Socket created");

    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(7000);

    //Bind
    if (bind(socket_file_descriptor, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        //print the error message
        perror("Bind failed. Error");
        return 1;
    }
    puts("Bind done");

    //Listen
    listen(socket_file_descriptor, 3);

    //Accept and incoming connection
    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);
    while ((client_sock = accept(socket_file_descriptor, (struct sockaddr *)&client, (socklen_t *)&c)))
    {
        puts("Connection accepted");

        pthread_t sniffer_thread;
        new_sock = malloc(1);
        *new_sock = client_sock;

        if (pthread_create(&sniffer_thread, NULL, client_provider, (void *)new_sock) < 0)
        {
            perror("Could not create thread");
            return 1;
        }

        pthread_join(sniffer_thread, NULL);
        puts("Handler assigned");
    }

    if (client_sock < 0)
    {
        perror("accept failed");
        return 1;
    }

    return 0;
}

void *client_provider(void *socket_file_descriptor)
{
    int loggedin = 0;
    int sock = *(int *)socket_file_descriptor;
    int read_size;
    char *message, input_command[1024];
    char id[SIZE_BUF], password[SIZE_BUF];
    char idpass[256];

    bzero(input_command, 1024 * sizeof(input_command[0]));
    while ((read_size = recv(sock, input_command, 1024, 0)) > 0)
    {
        if (strlen(input_command) == 0 || !(input_command[0] >= 33 && input_command[0] <= 126))
        {
            continue;
        }

        printf("%s\n", input_command);
        sprintf(database_query, "%s", input_command);

        if (run_with_segementation_fault_checking(run_db_command))
        {
            printf("Query Success!\n");
            send(sock, "Query Success!", 1024, 0);
        }
        else
        {
            printf("Query Error!\n");
            send(sock, "Query Error!", 1024, 0);
        }
        printf("\n\n");
        bzero(input_command, 1024 * sizeof(input_command[0]));
        bzero(message, 1024 * sizeof(message[0]));
    }
    if (read_size == 0)
    {
        puts("Client Disconnected\n");
    }
    else if (read_size == -1)
    {
        perror("Recv Failed\n");
    }

    close(sock);
    free(socket_file_descriptor);

    return 0;
}
