// mystore:  manages a linked-list database (called "mystore.dat") of little items

/* Implements commands:
   add
   stat
   display
   delete
   */

#define version "0.90"
#define author "PBrooks"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>

char *Usage = "Usage:\tmystore add \"subject\" \"body\"\n\
               mystore stat\n\
               mystore display {item-no}\n\
               mystore delete {item-no}\n\
               mystore edit {item-no} \"subject\" \"body\"\n";

#define NOTHING		0
#define ADD			1
#define STAT		2
#define DISPLAY		3
#define DELETE		4
#define EDIT		5

#define TRUE	1
#define FALSE	0

// Command line arguments processed:
int command = NOTHING;
char *subject = NULL;
char *body = NULL;
char *fields[5];
int item_start = -1;
int item_end = -1;

// Prototypes:
int handleArgs(char *message);
int parseArgs(char *message);
int isPositive(char *s);
int readData(void);
int add(char *subject, char *body);
void status(void);
char *rstrip(char *s);
void list(void);
static void the_handler(int sig);

// this describes the data item on disk
struct data {
    time_t theTime;
    char theSubject[31];
    char theBody[141];
};

// this describes the data item in memory (data with a link to the next carrier)
struct carrier {
    struct data theData;
    struct carrier *next;
};

// These variables track the data in memory
int nitems = 0;
struct carrier *first = NULL;
struct carrier *last = NULL;

int rewrite = FALSE;		// if data changes then rewrite
char errmsg[100] = "";

int fd_read, fd_write, fd_out;
char *fifo_read = "/tmp/mystore_server.dat";

// ---------------------------------- main() --------------------------------
int main(int argc, char **argv) {
    int how_much;
    char input[BUFSIZ];

    if (argc == 2) fifo_read = argv[1];

    printf("Capitalization FIFO server\nSend requests to: %s\n", fifo_read);
    printf("Requests:\nfifo_client {string}\necho print {string} >%s\necho return {client-fifo_filename} {string} >%s\n", fifo_read, fifo_read);
    printf("\nTerminate using \"kill -10 {pid of fifo_server}\"\n");
    printf("Can also be terminated with ^C\n");

    if (signal(SIGINT, the_handler) == SIG_ERR) {
        perror("Cannot set up signal handler on SIGINT...");
        return -1;
    }
    if (signal(SIGUSR1, the_handler) == SIG_ERR) {
        perror("Cannot set up signal handler on SIGUSR1...");
        return -1;
    }

    // remove the FIFO in case it exists
    unlink(fifo_read);

    // create the FIFO pseudo-file
    if (mkfifo(fifo_read, 0666) != 0) {
        perror("mkfifo error: ");
        return -1;
    }

    // open it for reading:
    fd_read = open(fifo_read, O_RDONLY);
    if (fd_read < 0) {
        printf("open(fifo_read) failed, returns: %d\n", fd_read);
        return fd_read;
    }

    // also open it for writing, but don't write anything to it (this is to prevent its closure when a client has finished writing to it).
    if ((fd_write = open(fifo_read, O_WRONLY)) < 0) {
        perror("Cannot open fifo_read for writing. ");
        return fd_write;
    }

    while (1) {
        how_much = read(fd_read, input, BUFSIZ);
        if (how_much > 0) {
            input[how_much]='\0';
            if (handleArgs(input) == -1) {
                printf("fifo_server quitting...\n");
                close(fd_read);
                unlink(fifo_read);
                return 0;
            }
        }
    }
    return 0;
}

int handleArgs(char *message) {
    if (!parseArgs(message)) {
        if (errmsg[0] != '\0')
            printf("%s\n", errmsg);
        else
            printf("|status: ERROR: No command-line arguments, or error in arguments\n\nVersion: %s\n%s|\n",
                    version, Usage);
        return 1;
    }

    if (!readData()) {
        if (errmsg[0] != '\0')
            printf("|status: ERROR: %s|\n", errmsg);
        else
            printf("|status: ERROR: Error reading mystore.dat\n\n%s|\n", Usage);
        return 1;
    }

    //All prints will go to the file
    if ((fd_out = open(fields[0], O_WRONLY)) < 0)
        printf("Cannot write to %s\n", fields[0]);

    if (command == ADD && !add(fields[2], fields[3])) {
        if (errmsg[0] != '\0')
            printf("|status: ERROR: %s|\n", errmsg);
        else
            printf("|status: ERROR: Failure to add new item|\n");
        return 1;
    }

    if (command == STAT) {
        status();
    }

    if (command == DISPLAY && !display(fields[2])) {
        if (errmsg[0] != '\0')
            printf("|status: ERROR: %s|\n", errmsg);
        else
            printf("|status: ERROR: Cannot display %s|\n", fields[2]);
        return 1;
    }

    if (command == DELETE && !delete(fields[2])) {
        if (errmsg[0] != '\0')
            printf("|status: ERROR: %s|\n", errmsg);
        else
            printf("|status: ERROR: Cannot delete %s|\n", fields[2]);
        return 1;
    }

    if (command == EDIT && !edit(fields[2])) {
        if (errmsg[0] != '\0')
            printf("|status: ERROR: %s|\n", errmsg);
        else
            printf("|status: ERROR: cannot edit %s|\n", fields[2]);
        return 1;
    }

    if (rewrite)
        if (!writeData()) {
            if (errmsg[0] != '\0')
                printf("|status: ERROR: %s|\n", errmsg);
            else
                printf("|status: ERROR: Could not write the data, file may be destroyed|\n");
            return 1;
        }

    close(fd_out);
    return 0;
}

// ------------------------------- parseArgs() -------------------------------
// parse the command-line arguments, and assign the global variables from them
// return FALSE if any problem with the command-line arguments
//
int parseArgs(char *message) {
    int nfields = SeparateIntoFields(message, fields, 5);
    if (nfields < 2) return FALSE;

    // try zero-argument commands: list and stat
    if (nfields == 2) {
        if (strcmp(fields[1], "stat") == 0) {
            command = STAT;
            return TRUE;
        }
        else {
            sprintf(errmsg, "Unrecognized argument: %s", fields[1]);
            return FALSE;
        }
    }
    // try the one-argument commands: delete and display
    else if (nfields == 3) {
        if (strcmp(fields[1],"delete") == 0 && isPositive(fields[2])) {
            command = DELETE;
            item_start = atoi(fields[2]);
            return TRUE;
        }
        else if (strcmp(fields[1], "display") == 0 && isPositive(fields[2])) {
            command = DISPLAY;
            item_start = atoi(fields[2]);
            return TRUE;
        }
        else {
            sprintf(errmsg, "Unrecognized 2-argument call: %s %s", fields[1], fields[2]);
            return FALSE;
        }
    }
    // try the two-argument command: add
    else if (nfields == 4) {
        if (strcmp(fields[1],"add") == 0) {
            command = ADD;
            subject = fields[2];
            body = fields[3];
            return TRUE;
        }
        else {
            sprintf(errmsg, "Unrecognized 3-argument call: %s %s %s", fields[1], fields[2], fields[3]);
            return FALSE;
        }
    }
    // try the three-argument command: edit
    else if (nfields == 5) {
        if (strcmp(fields[1], "edit") == 0 && isPositive(fields[2])) {
            command = EDIT;
            item_start = atoi(fields[2]);
            subject = fields[3];
            body = fields[4];
            return TRUE;
        }
        else {
            printf("Field 1: %s\n", fields[1]);
            printf("Field 2: %s\n", fields[2]);
            sprintf(errmsg, "Unrecognized 4-argument call: %s %s %s %s", fields[1], fields[2], fields[3], fields[4]);
            return FALSE;
        }
    }
    else
        return FALSE;
}

// ================================ SeparateIntoFields ===================================
int SeparateIntoFields(char *s, char **fields, int max_fields) {
    int i;
    static char null_c = '\0';
    int in_quote = 0;

    for (i = 0; i < max_fields; ++i) fields[i] = &null_c;

    for (i = 0; i < max_fields; ++i) {
        if (*s == '"' && !in_quote) {
            in_quote = 1;
            ++s;
        }
        while (*s && (*s == ' ' || *s == '\t' || *s == '\n')) ++s;	// skip whitespace
        if (!*s) return i;
        fields[i] = s;
        if (i == max_fields - 1) return i+1;
        while (*s && ((*s != ' ' && *s != '\t' && *s != '\n' && !in_quote) || (*s != '"' && in_quote))) {
            ++s;	// skip non-whitespace
        }
        in_quote = 0;
        if (!*s) return i+1;
        *s++ = '\0';
        while (*s && (*s == ' ' || *s == '\t' || *s == '\n')) ++s;	// skip whitespace
    }
    return -1;
}

// --------------------------------- isPositive ------------------------------
// return TRUE if the string is a positive (>= 1) integer
int isPositive(char *s) {
    char *p = s;
    while (*p >= '0' && *p <= '9') 
        ++p;
    if (p != s && *p == '\0' && atoi(s) > 0)
        return TRUE;
    return FALSE;
}

// ---------------------------------- readData --------------------------------
int readData(void) {
    int i;
    struct data current_data;
    struct carrier *current_carrier;
    struct carrier *previous_carrier;

    FILE *fp = fopen("mystore.dat", "rb");	// read in binary file mode
    if (!fp)
        return TRUE;	// no such file, that's OK: we're doing this for the first time

    if (fread(&nitems, sizeof(int), 1, fp) != 1) {  // try to read nitems
        fclose(fp);
        sprintf(errmsg, "Cannot read nitems");
        return FALSE;
    }

    for (i = 0; i < nitems; ++i) {
        if (fread(&current_data, sizeof(struct data), 1, fp) != 1) { //try to read the next item
            fclose(fp);
            sprintf(errmsg,"Cannot read item %d\n", i+1);
            return FALSE;
        }
        if ((current_carrier = calloc(1, sizeof(struct carrier))) == NULL) {  //allocate memory
            fclose(fp);
            sprintf(errmsg,"Cannot allocate %d\n", sizeof(struct carrier));
            return FALSE;
        }
        current_carrier->theData = current_data;	// load the data into the carrier
        // weave the linked-list
        if (i == 0)
            first = current_carrier;
        else
            previous_carrier->next = current_carrier;
        previous_carrier = current_carrier;
    }

    fclose(fp);
    last = current_carrier;
    return TRUE;
}

// ---------------------------------------- add --------------------------------------
int add(char *subject, char *body) {
    struct data current_data;
    struct carrier *current_carrier;

    // fill up the members of current_data
    strncpy(current_data.theSubject, subject, 30);
    current_data.theSubject[30]='\0';
    strncpy(current_data.theBody, body, 140);
    current_data.theSubject[140] = '\0';
    current_data.theTime = time(NULL);

    if ((current_carrier = calloc(1, sizeof(struct carrier))) == NULL) // allocate memory
        return FALSE;

    current_carrier->theData = current_data;
    if (nitems == 0)
        first = last = current_carrier;
    else {
        last->next = current_carrier;
        last = current_carrier;
    }

    ++nitems;
    rewrite = TRUE;
    char *all;
    asprintf(&all, "|status: OK|\n");
    write(fd_out, all, strlen(all));

    return TRUE;
}

// ------------------------------------- edit ------------------------------------
int edit(char *sn) {
    int n = atoi(sn);
    int i;
    struct carrier *ptr;
    struct data this_data;

    if (n > nitems) {
        sprintf(errmsg, "Cannot edit item %d.  Item numbers range from 1 to %d", n, nitems);
        return FALSE;
    }

    for (i = 1, ptr = first; i < n; ++i)
        ptr = ptr->next;

    this_data = ptr->theData;
    strncpy(this_data.theSubject, subject, 30);
    this_data.theSubject[30] = '\0';
    strncpy(this_data.theBody, body, 140);
    this_data.theBody[140] = '\0';
    this_data.theTime = time(NULL);
    ptr->theData = this_data;

    rewrite = TRUE;
    char *all;
    asprintf(&all, "|status: OK|\n");
    write(fd_out, all, strlen(all));
    return TRUE;
}

// ----------------------------------- writeData ---------------------------------
int writeData(void) {
    int i;
    struct carrier *ptr;
    struct data this_data;

    FILE *fp = fopen("mystore.dat", "wb");  // writing in binary
    if (!fp) {
        sprintf(errmsg, "Cannot open mystore.dat for writing.");
        return FALSE;
    }

    if (fwrite(&nitems, sizeof(int), 1, fp) != 1) {
        fclose(fp);
        sprintf(errmsg, "Cannot write the nitems element");
        return FALSE;
    }

    for (i = 0, ptr = first; i < nitems; ++i) {
        this_data = ptr->theData;
        if (fwrite(&this_data, sizeof(struct data), 1, fp) != 1) {
            fclose(fp);
            sprintf(errmsg, "Cannot write item: %d", i+1);
            return FALSE;
        }
        ptr = ptr->next;
    }

    fclose(fp);
    return TRUE;
}

// ------------------------------------- stat ------------------------------
void status(void) {
    struct tm *tp;
    char *status_str, *version_str, *author_str, *nitems_str, *first_time_str, *last_time_str;
    char *all;

    asprintf(&status_str, "|status: OK|\n");
    asprintf(&version_str, "|version: %s|\n", version);
    asprintf(&author_str, "|author: %s|\n", author);
    asprintf(&nitems_str, "|nitems: %d|\n", nitems);
    if (nitems != 0) {
        tp = localtime(&(first->theData.theTime));
        asprintf(&first_time_str, "|first-time: %d-%02d-%02d %02d:%02d:%02d|\n",
                tp->tm_year+1900, tp->tm_mon, tp->tm_mday, tp->tm_hour, tp->tm_min, tp->tm_sec);
        tp = localtime(&(last->theData.theTime));
        asprintf(&last_time_str, "|last-time: %d-%02d-%02d %02d:%02d:%02d|\n",
                tp->tm_year+1900, tp->tm_mon, tp->tm_mday, tp->tm_hour, tp->tm_min, tp->tm_sec);
        asprintf(&all, "%s%s%s%s%s%s", status_str, version_str, author_str, nitems_str, first_time_str, last_time_str);
    } else {
        asprintf(&all, "%s%s%s%s", status_str, version_str, author_str, nitems_str);
    }

    write(fd_out, all, strlen(all));
    return;
}

// ------------------------------------- rstrip ------------------------------
// removes the trailing whitespace 
char *rstrip(char *s) {
    char *p = s + strlen(s) - 1;
    while ((*p == ' ' || *p == '\t' || *p == '\n') && p >= s)
        --p;
    p[1] = '\0';
    return s;
}

// ------------------------------------ display -----------------------------
int display(char *sn) {
    int n = atoi(sn);
    int i;
    char *status_str, *item_str, *time_str, *subject_str, *body_str;
    struct carrier *ptr;
    struct data this_data;
    struct tm *tp;

    if (n > nitems) {
        sprintf(errmsg, "Cannot display item %d.  Item numbers range from 1 to %d", n, nitems);
        return FALSE;
    }

    for (i = 1, ptr = first; i < n; ++i)
        ptr = ptr->next;

    this_data = ptr->theData;
    asprintf(&status_str, "|status: OK|\n");
    asprintf(&item_str, "|item: %d|\n", n);
    tp = localtime(&this_data.theTime);
    asprintf(&time_str, "|time: %d-%02d-%02d %02d:%02d:%02d|\n",
            tp->tm_year+1900, tp->tm_mon, tp->tm_mday, tp->tm_hour, tp->tm_min, tp->tm_sec);
    asprintf(&subject_str, "|subject: %s|\n", this_data.theSubject);
    asprintf(&body_str, "|body: %s|\n", this_data.theBody);

    char *all;
    asprintf(&all, "%s%s%s%s%s", status_str, item_str, time_str, subject_str, body_str);
    write(fd_out, all, strlen(all) + 50);
    return TRUE;
}

// ------------------------------------ delete ------------------------------
int delete(char *sn) {
    int n = atoi(sn);
    int i;
    struct carrier *ptr, *previous;


    if (n > nitems) {
        sprintf(errmsg, "Cannot delete item %d.  Item numbers range from 1 to %d", n, nitems);
        return FALSE;
    }

    previous = first;
    if (n == 1) {
        first = first->next;
        if (nitems == 1) last = NULL;
    }
    else {
        for (i = 2, ptr = first->next; i < n; ++i) {
            previous = ptr;
            ptr = ptr->next;
        }
        previous->next = ptr->next;
        if (n == nitems) last = previous;
    }

    --nitems;
    rewrite = TRUE;
    char *all;
    asprintf(&all, "|status: OK|\n");
    write(fd_out, all, strlen(all));
    return TRUE;
}

// ============================ the_handler ==================
static void the_handler(int sig) {
    printf("Signal caught: fifo_server terminated by signal %d\n", sig);
    close(fd_read);
    close(fd_write);
    unlink(fifo_read);
    exit(0);
}
