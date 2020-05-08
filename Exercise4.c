#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "gmp.h"

gmp_randstate_t gmpRandState;

void convert_plaintext_to_ascii(mpz_t result, char *msg)
{
    int len = strlen(msg);

    // Καθώς ο χαρακτήρας πιάνει μία θέση μνήμης, ενώ ο ακέραιος ascii στον οποίο
    // αντιστοιχεί o χαρακτήρας 2-3 άρα το μέγεθος της συμβολοσειράς ascii_str θα είναι στο μέγιστο x3 το μέγεθος του αρχικού μηνύματος.
    char *ascii_str = malloc(3 * len);
    char ascii_char[3];

    for (int i = 0; i < len; i++) {
        // Μετατροπή ενός χαρακτήρα του μηνύματος σε έναν ακέραιο πχ h -> 104.
        int ascii_int = msg[i];

        // Αντιστοιχούμε τον χαρακτήρα '\n' με το 30.
        if (ascii_int == 10) ascii_int = 30;

        // Μετατροπή του ακεραίου σε μία ακολουθία τριών χαρακτήρων πχ 104 -> "104".
        sprintf(ascii_char, "%d", ascii_int);

        // Εισαγωγή των χαρακτήρων στον πίνακα ascii_str.
        if (i == 0) strcpy(ascii_str, ascii_char);
        else strcat(ascii_str, ascii_char);
    }

    char *tmp = realloc(ascii_str, strlen(ascii_str) + 1);
    if (tmp == NULL) printf("Realloc has failed, program will exit now.\n"), exit(EXIT_FAILURE);
    else ascii_str = tmp;

    // Μετατροπή της συμβολοσειράς ascii_str σε έναν ακέραιο mpz_t
    mpz_set_str(result, ascii_str, 10);
}

char *convert_ascii_to_plaintext(mpz_t param)
{
    int ascii_msg_len = mpz_sizeinbase(param, 10);
    int asci_int, cnt = 0;

    char *ascii_msg = malloc(ascii_msg_len + 1);
    char *final_msg = malloc(ascii_msg_len + 1);
    char c, ascii_char[3];

    // Μετατροπή του ακεραίου mpz_t σε συμβολοσειρά.
    mpz_get_str(ascii_msg, 10, param);

    for (int i = 0; i < ascii_msg_len; i = i + 2) {
        // Αντιγραφή δύο ακεραίων από την ακολουθία ascii στον πίνακα ascii_char.
        strncpy(ascii_char, ascii_msg + i, 2);

        // Μετατροπή των ακεραίων σε μορφή χαρακτήρα σε έναν ακεραίο πχ "65" -> 65.
        asci_int = atoi(ascii_char);

        // Αν ο ακέραιος είναι μικρότερος του 32 αντιγράφουμε ακόμα έναν χαρακρτήρα.
        if (asci_int < 30) {
            strncpy(ascii_char, ascii_msg + i, 3);
            i += 1;

            asci_int = atoi(ascii_char);
        }

            // Επαναφορά της τιμής '\n' που προηγουμένως είχε αλλάξει σε 30.
        else if (asci_int == 30) asci_int = 10;

        // Μετατροπή του ακεραίου σε χαρακτήρα πχ "97" -> a.
        c = (char) asci_int;

        // Αποθήκευση του χαρακτήρα στην τελική συμβολοσειρά.
        final_msg[cnt++] = c;

        // Εισαγωγή του χαρακτήρα '\0' στο τελευταίο κελί έτσι ώστε τα αποτελέσματα από τον προηγούμενο χαρακτήρα να μην επηρεάσουν τον επόμενο.
        ascii_char[2] = '\0';
    }

    final_msg[cnt] = '\0';

    char *tmp = realloc(final_msg, strlen(final_msg) + 1);
    if (tmp == NULL) printf("Realloc has failed, program will exit now.\n"), exit(EXIT_FAILURE);
    else final_msg = tmp;

    return final_msg;
}

void gen_keys(mpz_t n, mpz_t e, mpz_t d, int bits)
{
    mpz_t rand_num, p, q, phi;
    mpz_inits(rand_num, p, q, phi, NULL);

    // Δημιουργία ενός τυχαίου αριθμού όπου το μέγεθος του σε bit δίνεται με όρισμα.
    mpz_urandomb(rand_num, gmpRandState, bits);

    // Οι πρώτοι αριθμοί p,q είναι οι επόμενοι 2 πρώτοι μετά τον τυχαίο αριθμό.
    mpz_nextprime(p, rand_num);
    mpz_nextprime(q, p);

    // Υπολογισμός της τιμής n = p*q
    mpz_mul(n, p, q);

    mpz_sub_ui(p, p, 1);
    mpz_sub_ui(q, q, 1);

    // φ = (p-1) * (q-1)
    mpz_mul(phi, p, q);

    // Επανάληψη μέχρι να βρεθεί ένα e το οποίο να είναι αντιστρέψιμο στο Zφ.
    do
    {
        mpz_urandomb(rand_num, gmpRandState, 7);
        mpz_nextprime(e, rand_num);

        // Υπολογισμός του d o οποίος είναι ο αντίστροφος του e στο Ζφ.
        mpz_invert(d, e, phi);
    }
    while (mpz_cmp_ui(d, 0) < 1);

    mpz_clears(rand_num, p, q, phi, NULL);
}

void encryptMsg(mpz_t c, mpz_t n, mpz_t e, char *plaintext)
{
    mpz_t ascii_plaintext;
    mpz_init(ascii_plaintext);

    // Μετατροπή του κειμένου σε μία ακολουθία ascii.
    convert_plaintext_to_ascii(ascii_plaintext, plaintext);

    // Το πρόγραμμα τερματίζει εάν το μήνυμα ξεπερνάει το εύρος [0, n-1].
    if (mpz_cmp(ascii_plaintext, n) >= 0)
    {
        gmp_printf("your message exceeds maximum message length: %Zd \n", n);
        exit(EXIT_FAILURE);
    }

    // Υπολογισμός της τιμής c = m^e mod n
    mpz_powm(c, ascii_plaintext, e, n);
    mpz_clear(ascii_plaintext);
}

char *decryptMsg(mpz_t c, mpz_t d, mpz_t n)
{
    mpz_t decr_ascii_plaintext;
    mpz_init(decr_ascii_plaintext);

    // Υπολογισμός της τιμής m = c^d mod n
    mpz_powm(decr_ascii_plaintext, c, d, n);

    // Μετατροπή του μηνύματος από ascii στο αρχικό κείμενο.
    char *msg = convert_ascii_to_plaintext(decr_ascii_plaintext);

    mpz_clear(decr_ascii_plaintext);
    return msg;
}

void *myRealloc(void *ptr, int len)
{
    void *tmp = realloc(ptr, len);
    if (tmp == NULL) printf("Realloc has failed, program will exit now.\n"), exit(EXIT_FAILURE);
    else ptr = tmp;

    return ptr;
}

char *getLine(FILE *fp)
{
    int sizeof_line = 10; // τρέχον μέγεθος του δείκτη len
    int len = 0; // Θέση της γραμμής που διαβάζουμε
    int min_size = 10; // Ελάχιστο περιθώριο του δείκτη len
    char *line = malloc(sizeof_line);
    char ch;

    // Εάν έχουμε φτάσει στο τέλος του αρχείου επιστρέφουμε NULL.
    if (feof(fp)) return NULL;

    while ((ch = fgetc(fp)) != EOF && (ch != '\n'))
    {
        line[len++] = ch;

        // Εάν η γραμμή του αρχείου ξεπεράσει το μέγεθος που έχουμε δώσει κάνουμε realloc και ενημερώνουμε την τιμή του sizeof_line
        if (len == sizeof_line)
            line = myRealloc(line, len + min_size + 1),
            sizeof_line += min_size;
    }

    line = myRealloc(line, len + 1);
    line[len] = '\0';

    return line;
}

int main()
{
    srand(time(NULL));
    gmp_randinit(gmpRandState, GMP_RAND_ALG_LC, 120);
    gmp_randseed_ui(gmpRandState, rand());

    mpz_t tmp_key, ciphertext_line, n, e, d, c;
    mpz_inits(tmp_key, ciphertext_line, n, e, d, c, NULL);

    printf("Select option 1-5\nCreate or read keys before encrypting\\decrypting\n");
    int ans = 0;

    while (1)
    {
        printf("1. Create New Keys \n");
        printf("2. Read Keys \n");
        printf("3. Encrypt File \n");
        printf("4. Decrypt File \n");
        printf("5. Exit \n>>");
        scanf("%d", &ans);

        // Αν ο χρήστης δεν έχει δημιουργήσει κλειδιά του λέμε να τα δημιουργήσει.
        if ((ans == 3 || ans == 4) && mpz_cmp_ui(n, 0) == 0)
        {
            printf("You must first create keys before encrypting\\decrypting\n\n");
            continue;
        }

        // Δημιουργία δημοσίων και ιδιωτικών κλειδιών n,e,d και εγγραφή τους στο αρχείο keys.txt
        if (ans == 1)
        {
            // Άνοιγμα του αρχείου keys.
            FILE *fp = fopen(".\\keys.txt", "w+");
            char *tmp = malloc(100);

            // Δημιουργία δημόσιου και ιδιωτικού κλειδιού 512 bit.
            gen_keys(n, e, d, 512);

            // Εγγραφή των κλειδιών στο αρχείο keys.txt
            for (int i = 0; i < 3; i++)
            {
                // Καταχώρηση ενός κλειδιού στην μεταβλητή tmp_key.
                if (i == 0) mpz_set(tmp_key, n);
                else if (i == 1) mpz_set(tmp_key, e);
                else if (i == 2) mpz_set(tmp_key, d);

                // Μετατροπή της mpz μεταβλητής σε string.
                tmp = realloc(tmp, mpz_sizeinbase(tmp_key, 10) + 1);
                mpz_get_str(tmp, 10, tmp_key);

                // Καταχώρηση του κλειδιού στο αρχείο keys.
                fprintf(fp, tmp);
                fprintf(fp, "\n\n");
            }

            fclose(fp); // Κλείνουμε το αρχείο.

            gmp_printf("Created keys:\nn: %Zu \n\n", n);
            gmp_printf("e: %Zu \n\n", e);
            gmp_printf("d: %Zu \n\n", d);
        }

        // Διάβασμα των δημοσίων και ιδιωτικών κλειδιών από το αρχείο keys.txt και καταχώρηση τους στις αντίστοιχες μεταβλητές.
        else if (ans == 2)
        {
            // Άνοιγμα του αρχείου keys.
            FILE *fp = fopen(".\\keys.txt", "r");
            char *line = NULL;
            int i = 0;

            if (fp == NULL)
            {
                printf("Keys file does not exist.\n\n"); continue;
            }

            // Διαβάζουμε μία γραμμή από το αρχείο.
            while((line = getLine(fp)) != NULL)
            {
                // Αν η γραμμή περιέχει κάποιο κλειδί το καταχωρούμε στην αντίστοιχη μεταβλητή.
                if (*line != '\0')
                {
                    if (i == 0) mpz_set_str(n, line, 10), i++;
                    else if (i == 1) mpz_set_str(e, line, 10), i++;
                    else if (i == 2) mpz_set_str(d, line, 10), i++;
                }
            }
            fclose(fp); // Κλείνουμε το αρχείο.

            gmp_printf("Keys from file keys.txt:\nn: %Zu \n\n", n);
            gmp_printf("e: %Zu \n\n", e);
            gmp_printf("d: %Zu \n\n", d);
        }

        // Κρυπτογράφηση του αρχείου plaintext.txt
        else if (ans == 3)
        {
            // Άνοιγμα των αρχείων plaintext και ciphertext.
            FILE *fp1 = fopen(".\\plaintext.txt", "r");
            FILE *fp2 = fopen(".\\ciphertext.txt", "w+");
            char *plaintext_line = NULL, *cipher_line = malloc(50);

            if (fp1 == NULL)
            {
                printf("Plaintext file does not exist.\n"); continue;
            }

            printf("Encrypting file: \n");

            // Διαβάζουμε μία γραμμή από το αρχείο plaintext.
            while ((plaintext_line = getLine(fp1)) != NULL)
            {
                if (plaintext_line[0] != '\0')
                {
                    // Κρυπτογράφηση της γραμμής.
                    encryptMsg(c, n, e, plaintext_line);

                    cipher_line = myRealloc(cipher_line, mpz_sizeinbase(c, 10) + 1);

                    // Μετατροπή του ciphertext σε string πχ 16753567 -> "16753567".
                    mpz_get_str(cipher_line, 10, c);

                    // Καταχώρηση του ciphertext στο αρχείο ciphertext.txt
                    fprintf(fp2, cipher_line);

                    printf("line in plaintext: %s \n", plaintext_line);
                    printf("line in ciphertext: %s \n\n", cipher_line);
                }
                fprintf(fp2, "\n");
            }

            // Κλείνουμε τα αρχεία.
            fclose(fp1);
            fclose(fp2);

            printf("File encrypted \n\n");
        }

        else if (ans == 4)
        {
            // Άνοιγμα των αρχείων ciphertext και decrypted_plaintext.
            FILE *fp1 = fopen(".\\ciphertext.txt", "r");
            FILE *fp2 = fopen(".\\decrypted_plaintext.txt", "w+");
            char *cipher_line = NULL;

            if (fp1 == NULL)
            {
                printf("Ciphertext file does not exist.\n"); continue;
            }

            printf("Decrypting file: \n");

            // Διαβάζουμε μία γραμμή από το αρχείο ciphertext.
            while ((cipher_line = getLine(fp1)) != NULL)
            {
                if (cipher_line[0] != '\0')
                {
                    mpz_set_str(ciphertext_line, cipher_line, 10);

                    // Αποκρυπτογράφηση της γραμμής.
                    char *plaintext_line = decryptMsg(ciphertext_line, d, n);

                    // Καταχώρηση της plaintext γραμμής στο αρχείο decrypted_plaintext
                    fprintf(fp2, plaintext_line);

                    printf("line in ciphertext: %s \n", cipher_line);
                    printf("line in plaintext: %s \n\n", plaintext_line);
                }
                fprintf(fp2, "\n");
            }

            // Κλείνουμε τα αρχεία.
            fclose(fp1);
            fclose(fp2);

            printf("File decrypted \n\n");
        }

        else if (ans == 5) break;
    }

    gmp_randclear(gmpRandState);
    mpz_clears(tmp_key, ciphertext_line, n, e, d, c, NULL);

    return 0;
}