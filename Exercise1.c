#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "gmp.h"

gmp_randstate_t gmpRandState;

void convert_plaintext_to_ascii(mpz_t ascii_plaintext, char *plaintext)
{
    int len = strlen(plaintext);

    // Καθώς ο χαρακτήρας πιάνει μία θέση μνήμης, ενώ ο ακέραιος ascii στον οποίο
    // αντιστοιχεί o χαρακτήρας 2-3 άρα το μέγεθος της συμβολοσειράς ascii_str θα είναι στο μέγιστο x3 το μέγεθος του αρχικού μηνύματος.
    char *ascii_str = malloc(3 * len);
    char ascii_char[3];

    for (int i = 0; i < len; i++) {
        // Μετατροπή ενός χαρακτήρα του μηνύματος σε έναν ακέραιο πχ h -> 104.
        int ascii_int = plaintext[i];

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
    mpz_set_str(ascii_plaintext, ascii_str, 10);
}

char *convert_ascii_to_plaintext(mpz_t ascii_plaintext)
{
    int ascii_msg_len = mpz_sizeinbase(ascii_plaintext, 10);
    int asci_int, cnt = 0;

    char *ascii_msg = malloc(ascii_msg_len + 1);
    char *plaintext = malloc(ascii_msg_len + 1);
    char c, ascii_char[3];

    // Μετατροπή του ακεραίου mpz_t σε συμβολοσειρά.
    mpz_get_str(ascii_msg, 10, ascii_plaintext);

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
        plaintext[cnt++] = c;

        // Εισαγωγή του χαρακτήρα '\0' στο τελευταίο κελί έτσι ώστε τα αποτελέσματα από τον προηγούμενο χαρακτήρα να μην επηρεάσουν τον επόμενο.
        ascii_char[2] = '\0';
    }

    plaintext[cnt] = '\0';

    char *tmp = realloc(plaintext, strlen(plaintext) + 1);
    if (tmp == NULL) printf("Realloc has failed, program will exit now.\n"), exit(EXIT_FAILURE);
    else plaintext = tmp;

    return plaintext;
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
    gmp_printf("plaintext in ascii: %Zd \n", ascii_plaintext);

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
    gmp_printf("ascii plaintext after decryption: %Zd \n", decr_ascii_plaintext);

    // Μετατροπή του μηνύματος από ascii στο αρχικό κείμενο.
    char *msg = convert_ascii_to_plaintext(decr_ascii_plaintext);

    mpz_clear(decr_ascii_plaintext);
    return msg;
}

int main()
{
    srand(time(NULL));
    gmp_randinit(gmpRandState, GMP_RAND_ALG_LC, 120);
    gmp_randseed_ui(gmpRandState, rand());

    mpz_t n, e, d, c, m1, m2;
    mpz_inits(n, e, d, c, m1, m2, NULL);

    // Εισαγωγή μηνύματος εδώ.
    char *plaintext = "Cryptography is the practice of techniques for communication in the presence of third parties called adversaries";

    // Δημιουργία δημόσιου και ιδιωτικού κλειδιού 512 bit.
    gen_keys(n, e, d, 512);

    printf("public key --------------------------------------------------------------------------------------------\n");
    gmp_printf("n: %Zd\n", n);
    gmp_printf("e: %Zd\n\n", e);

    printf("private key -------------------------------------------------------------------------------------------\n");
    gmp_printf("d: %Zd\n\n", d);

    printf("encryption --------------------------------------------------------------------------------------------\n");
    printf("plaintext: %s \n", plaintext);

    encryptMsg(c, n, e, plaintext);
    gmp_printf("ciphertext: %Zd\n\n", c);

    printf("decryption --------------------------------------------------------------------------------------------\n");
    char *decrypted_plaintext = decryptMsg(c, d, n);
    printf("plaintext after decryption: %s \n", decrypted_plaintext);

    gmp_randclear(gmpRandState);
    mpz_clears(n, e, d, c, m1, m2, NULL);

    return 0;
}