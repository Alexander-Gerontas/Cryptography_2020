#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "gmp.h"

gmp_randstate_t gmpRandState;

void convert_msg_to_int(mpz_t result, char *msg)
{
    int len = strlen(msg);

    // Καθώς ο χαρακτήρας πιάνει μία θέση μνήμης, ενώ ο ακέραιος ascii στον οποίο
    // αντιστοιχεί 2-3 άρα το μέγεθος της συμβολοσειράς ascii_str θα είναι x3 το μέγεθος του αρχικού μηνύματος
    char *ascii_str = malloc(3 * len);
    char ascii_char[3];

    for (int i = 0; i < len; i++) {
        // Μετατροπή ενός χαρακτήρα του μηνύματος σε έναν ακέραιο πχ h -> 104.
        int asci_int = msg[i];

        // Αντιστοιχούμε τον χαρακτήρα '\n' με το 30.
        if (asci_int == 10) asci_int = 30;

        // Μετατροπή του ακεραίου σε μία ακολουθία τριών χαρακτήρων πχ 104 -> "104".
        itoa(asci_int, ascii_char, 10);

        if (i == 0) strcpy(ascii_str, ascii_char);
        else strcat(ascii_str, ascii_char);
    }

    // Μετατροπή της συμβολοσειράς ascii_str σε έναν ακέραιο mpz_t
    mpz_set_str(result, ascii_str, 10);
}

char *convert_int_to_msg(mpz_t param)
{
    int ascii_msg_len = mpz_sizeinbase(param, 10);
    int asci_int, final_msg_length, cnt = 0;

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

        // Επαναφορά της τιμής '\n' που προηγουμένος είχε αλλάξει σε 30.
        else if (asci_int == 30) asci_int = 10;

        // Μετατροπή του ακεραίου σε χαρακτήρα πχ "97" -> a.
        c = (char) asci_int;

        // Αποθήκευση του χαρακτήρα στην τελική συμβολοσειρά.
        final_msg[cnt++] = c;

        // Εισαγωγή του χαρακτήρα '\0' στο τελευταίο κελί έτσι ώστε τα αποτελέσματα από τον προηγούμενο χαρακτήρα να μην επηρεάσουν τον επόμενο.
        ascii_char[2] = '\0';
    }

    final_msg[cnt] = '\0';
    final_msg_length = strlen(final_msg);

    final_msg = (char *) realloc(final_msg, final_msg_length + 1);

    return final_msg;
}

void gen_keys(mpz_t n, mpz_t e, mpz_t d, int bits)
{
    int p_bits;

    mpz_t tmp, rand_num, p1, p2, phi;
    mpz_inits(tmp, rand_num, p1, p2, phi, NULL);

    mpz_init_set_ui(tmp, 47);

//    mpz_urandomb(rand_num, gmpRandState, 47);
    mpz_urandomb(rand_num, gmpRandState, bits);
//    mpz_urandomm(rand_num, gmpRandState, tmp);

    gmp_printf("rand num: %Zd ------------------------\n", rand_num);

    mpz_nextprime(p1, rand_num);
    mpz_nextprime(p2, p1);

    gmp_printf("p1: %Zd ------------------------------ \n", p1);

    p_bits = mpz_sizeinbase(p1, 2);
    printf("p1 bits_2: %d\n", p_bits);
//    printf("p1 bits in base 2: %d\n", p_bits);

    p_bits = mpz_sizeinbase(p1, 10);
    printf("p1 bits_10: %d\n", p_bits);
//    printf("p1 bits in base 10: %d\n", p_bits);

    mpz_mul(n, p1, p2);
    gmp_printf("n: %Zd -------------------------------\n", n);

    int n_bits_base_2, n_bits_base_10;

    n_bits_base_2 = mpz_sizeinbase(n, 2);
//    printf("n bits in base 2: %d\n", n_bits_base_2);
    printf("n bits_2: %d\n", n_bits_base_2);

    n_bits_base_10 = mpz_sizeinbase(n, 10);
    printf("n bits_10: %d\n", n_bits_base_10);
//    printf("n bits in base 10: %d\n", n_bits_base_10);

    mpz_sub_ui(p1, p1, 1);
    mpz_sub_ui(p2, p2, 1);

    mpz_mul(phi, p1, p2);

    mpz_invert(d, e, phi);

//    mpz_clears(p1, p2, 0);
    mpz_clears(tmp, rand_num, p1, p2, phi, NULL);
}

void decryptMsg(mpz_t res, mpz_t c, mpz_t d, mpz_t n)
{
    mpz_powm(res, c, d, n);
}

int main()
{
    mpz_t n, e, d, c, m1, m2;
    mpz_inits(n, e, d, c, m1, m2, NULL);

    //    char *msg = "hello";
        char *msg = "hello my name is alex";
//    char *msg = "hi my name is alex\n"
//                "this is a sentence sentence \n"
//                "this is another sentence \n";

    printf("orisinal msg: %s \n", msg);

    convert_msg_to_int(m1, msg);
    gmp_printf("m1 msg: %Zd -------------------------- \n", m1);

    mpz_init_set_ui(e, 101);
    mpz_nextprime(e, e);

    srand(time(NULL));
//    gmp_randinit_default(gmpRandState);
    gmp_randinit(gmpRandState, GMP_RAND_ALG_LC, 120);
    gmp_randseed_ui(gmpRandState, rand());

    int msg_bits_base_10, msg_bits_base_2;

    msg_bits_base_2 = mpz_sizeinbase(m1, 2);
    printf("msg bits_2: %d\n", msg_bits_base_2);
//    printf("msg bits in base 2: %d\n", msg_bits_base_2);

    msg_bits_base_10 = mpz_sizeinbase(m1, 10);
    printf("msg bits_10: %d\n", msg_bits_base_10);
//    printf("msg bits in base 10: %d\n", msg_bits_base_10);

//    gen_keys(n, e, d, 3*msg_bits_base_2 / 2);
//    gen_keys(n, e, d, 4 * msg_bits_base_2 / 3);
//    gen_keys(n, e, d, msg_bits_base_2 - 15);
    gen_keys(n, e, d, msg_bits_base_2);

    mpz_powm(c, m1, e, n);
    mpz_powm(m2, c, d, n);

    gmp_printf("m2 msg: %Zd \n", m2);

    char *new_msg = convert_int_to_msg(m2);
    printf("msg after decryption: %s \n", new_msg);

    gmp_randclear(gmpRandState);
    mpz_clears(n, e, d, c, m1, m2, NULL);

    return 0;
}