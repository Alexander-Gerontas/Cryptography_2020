#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "gmp.h"

gmp_randstate_t gmpRandState;

void convert_msg_to_int(mpz_t result, char *msg)
{
    int len = strlen(msg);
    printf("len msg: %d\n", len);

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
        itoa(ascii_int, ascii_char, 10);

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

char *convert_int_to_msg(mpz_t param)
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

void gen_keys(mpz_t p, mpz_t a, mpz_t a_d, mpz_t d, int bit)
{
    mpz_t rand_num, f_n, tmp;
    mpz_inits(rand_num, f_n, tmp, NULL);

    mpz_urandomb(rand_num, gmpRandState, bit);

    mpz_nextprime(p, rand_num);

    // καθως ο p ειναι πρωτος η πολλ ομαδα του p Zn* = {1 - p-1} diaf 4, 9

    mpz_sub_ui(f_n, p, 1);

    mpz_div_ui(a, p, 2); // θα μπορουσε μια random να καθωριζει την διαιρεση /2 - /5

    while (1)
    {
        mpz_add_ui(a, a, 1);

        mpz_powm(tmp, a, f_n, p);

        if (mpz_cmp_ui(tmp, 1) == 0)
            break;
    }

    mpz_set_ui(d, 10);

    mpz_pow_ui(a_d, a, mpz_get_ui(d));

    mpz_clears(rand_num, f_n, tmp, NULL);
}

void encryptMsg(mpz_t gamma, mpz_t delta, mpz_t p, mpz_t a, mpz_t a_d, mpz_t gmp_ascii_msg)
{
    mpz_t tmp, k;
    mpz_inits(tmp, k, NULL);

    mpz_set_ui(k, 100);

    mpz_powm(gamma, a, k, p);

    mpz_pow_ui(tmp, a_d, mpz_get_ui(k));
    mpz_mul(tmp, tmp, gmp_ascii_msg);

    mpz_mod(delta, tmp, p);

    mpz_clears(tmp, k, NULL);
}

void decryptMsg(mpz_t m2, mpz_t p, mpz_t d, mpz_t gamma, mpz_t delta)
{
    printf("decrypt msg func ----------------------------------------\n");

    gmp_printf("gamma: %Zd\n", gamma);

    mpz_t tmp, g1, g2;
    mpz_inits(tmp, g1, g2, NULL);

    // tmp = p-1-d
    mpz_sub_ui(tmp, p, 1);
    mpz_sub(tmp, tmp, d);

    // g1 = γ^(p-1-d) mod p
    mpz_powm(g1, gamma, tmp, p);
    gmp_printf("g1: %Zd\n", g1);

    // g2 = γ^-d
//    mpz_neg(d, d);

//    mpz_set_si(d, -10);
    gmp_printf("d: %Zd\n", d);

    mpz_pow_ui(g2, gamma, mpz_get_ui(d));
//    mpz_pow_ui(g2, gamma, -10);
    gmp_printf("g2: %Zd\n", g2);

    // g2 = γ^-d mod p
    mpz_powm(g2, gamma, d, p);
    gmp_printf("g2: %Zd\n", g2);

    // m2 = (γ^-d) * δ mod p
    mpz_mul(tmp, g1, delta);
    mpz_mod(m2, tmp, p);

    mpz_clears(tmp, g1, g2, NULL);
}

int main()
{
    srand(time(NULL));
    gmp_randinit(gmpRandState, GMP_RAND_ALG_LC, 120);

    mpz_t p, a, a_d, d, gamma, delta, ascii_msg, m2;
    mpz_inits(p, a, a_d, d, gamma, delta, ascii_msg, m2, NULL);

    // Εισαγωγή μηνύματος εδώ.
//    char *msg = "i play pokemon go\n";
//    char *msg = "hello";
//    char *msg = "HELLO";
    char *msg = "test message\n";
    printf("original msg: %s \n", msg);

    gmp_randseed_ui(gmpRandState, rand());

    // Δημιουργία ιδιωτικού και δημοσίου κλειδιού 200 bit.
    gen_keys(p, a, a_d, d, 200);

//    printf("public key ---------------------------------------------------------\n");
//    gmp_printf("p: %Zd\n", p);
//    gmp_printf("a: %Zd\n", a);
//    gmp_printf("a^d: %Zd\n", a_d);

//    printf("private key ---------------------------------------------------------\n");
//    gmp_printf("d: %Zd\n", d);

    convert_msg_to_int(ascii_msg, msg);

    if (mpz_sizeinbase(p, 2) < mpz_sizeinbase(ascii_msg, 2))
    {
//        printf("your message exceeds maximum message length \n");
//        printf("message length in bits: %zu \n", mpz_sizeinbase(ascii_msg, 2));
//        printf("maximum message length: %zu \n", mpz_sizeinbase(p, 2));
        mpz_clears(p, a, a_d, d, gamma, delta, ascii_msg, m2, NULL);
        exit(EXIT_FAILURE);
    }

    gmp_printf("gmp msg before encryption %Zd\n", ascii_msg);

    encryptMsg(gamma, delta, p, a, a_d, ascii_msg);

    decryptMsg(m2, p, d, gamma, delta);

//    gmp_printf("gmp msg after decryption %Zd\n", ascii_msg);
//    gmp_printf("gmp msg after decryption %Zd\n", m2);

    char *new_msg = convert_int_to_msg(m2);
    printf("new msg: %s \n", new_msg);

//    gmp_randclear(gmpRandState);
    mpz_clears(p, a, a_d, d, gamma, delta, ascii_msg, m2, NULL);

    return 0;
}