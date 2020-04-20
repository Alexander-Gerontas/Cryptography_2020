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

void gen_keys(mpz_t n, mpz_t p, mpz_t q, int bit)
{
    mpz_t rand_num, tmp1, tmp2;
    mpz_inits(rand_num, tmp1, tmp2, NULL);

    while (1)
    {
        mpz_urandomb(rand_num, gmpRandState, bit);

        mpz_nextprime(p, rand_num);
        mpz_nextprime(q, p);

        mpz_mod_ui(tmp1, p, 4);
        mpz_mod_ui(tmp2, q, 4);

        if (mpz_cmp(tmp1, tmp2) == 0 && mpz_cmp_ui(tmp2, 3) == 0)
        {
//            printf("mod 4 true\n");
            break;
        }
    }

    gmp_printf("p %Zd\n", p);
    gmp_printf("q %Zd\n", q);

    mpz_mul(n, p, q);

    mpz_clears(rand_num, tmp1, tmp2, NULL);
}

void roots(mpz_t arr[], mpz_t r1, mpz_t r2, mpz_t r3, mpz_t r4, mpz_t n, mpz_t p, mpz_t q, mpz_t c)
{
    mpz_t g, a, b, p1, q1, r, s, tmp, tmp1, tmp2;
    mpz_inits(g, a, b, p1, q1, r, s, tmp, tmp1, tmp2, NULL);

    mpz_gcdext(g, a, b, p, q);

    printf("roots func ----------------------------------------\n");

    gmp_printf("g: %Zd\n", g);
    gmp_printf("a: %Zd\n", a);
    gmp_printf("b: %Zd\n", b);

    // test ap * bq
    printf("gen tmp1: \n");

    // Δημιουργία p1, q1 όπου p1 = (p+1)/4
    mpz_add_ui(tmp1, p, 1);
    mpz_add_ui(tmp2, q, 1);

    mpz_div_ui(p1, tmp1, 4);
    mpz_div_ui(q1, tmp2, 4);

    mpz_powm(r, c, p1, p);
    mpz_powm(s, c, q1, q);

    gmp_printf("n: %Zd\n", n);
    gmp_printf("p: %Zd\n", p);
    gmp_printf("q: %Zd\n", q);
    gmp_printf("p1: %Zd\n", p1);
    gmp_printf("q1: %Zd\n", q1);
    gmp_printf("c: %Zd\n", c);
    gmp_printf("r: %Zd\n", r);
    gmp_printf("s: %Zd\n", s);

    mpz_mul(tmp, a, p);
    mpz_mul(tmp1, tmp, s);

    mpz_mul(tmp, b, q);
    mpz_mul(tmp2, tmp, r);

    gmp_printf("tmp1 (aps): %Zd\n", tmp1);
    gmp_printf("tmp2 (bqr): %Zd\n", tmp2);

    /*
    // υπολογσιμος x - 1η ρίζα (r1)
    mpz_add(tmp, tmp1, tmp2);
    mpz_mod(r1, tmp, n);

    // υπολογσιμος y - 2η ρίζα (r2)
    mpz_sub(tmp, tmp1, tmp2);
    mpz_mod(r2, tmp, n);

//    gmp_printf("r1 neg: %Zd\n", tmp);
    mpz_mod(tmp, r1, n);
    mpz_neg(r3, tmp);

    // 4h ρίζα
//    gmp_printf("r2 neg: %Zd\n", r4);
    mpz_mod(tmp, r2, n);
    mpz_neg(r4, tmp);

    */

    mpz_add(tmp, tmp1, tmp2);
    mpz_mod(r1, tmp, n);

    mpz_sub(r2, n, r1);
//    mpz_mod(r1, tmp, n);

    mpz_sub(tmp, tmp1, tmp2);
    mpz_mod(r3, tmp, n);

    mpz_sub(r4, n, r3);

    gmp_printf("r1: %Zd\n", r1);
    gmp_printf("r2: %Zd\n", r2);
    gmp_printf("r3: %Zd\n", r3);
    gmp_printf("r4: %Zd\n", r4);

//    mpz_t arr[4];
    mpz_set(arr[0], r1);
    mpz_set(arr[1], r2);
    mpz_set(arr[2], r3);
    mpz_set(arr[3], r4);

    mpz_clears(g, a, b, p1, q1, r, s, tmp, tmp1, tmp2, NULL);
}

void encrytMsg(mpz_t c, mpz_t gmp_ascii_msg, mpz_t n)
{
    char *bin_msg, last_bits[7];

    int len = mpz_sizeinbase(gmp_ascii_msg, 2);

    printf("dec msg 1: "); mpz_out_str(stdout, 10, gmp_ascii_msg); printf("\n");
    printf("bin msg 1: "); mpz_out_str(stdout, 2, gmp_ascii_msg); printf("\n");

    // μετατροπή του μηνύματος σε δυαδικό
    bin_msg = malloc(len + 7);
    mpz_get_str(bin_msg, 2, gmp_ascii_msg);

    printf("bin msg 1: %s \n", bin_msg);

    // Αντιγραφή των τελευταίων 6 bit του μηνύματος στην μεταβλητή last_bits
    strncpy(last_bits, bin_msg + len - 6, 6);

    // Προσθήκη των 6 bit στο τέλος του μηνύματος.
    strcat(bin_msg, last_bits);

    mpz_init_set_str(gmp_ascii_msg, bin_msg, 2);

    printf("dec msg 2: "); mpz_out_str(stdout, 10, gmp_ascii_msg); printf("\n");
    printf("bin msg 2: "); mpz_out_str(stdout, 2, gmp_ascii_msg); printf("\n");

    mpz_powm_ui(c, gmp_ascii_msg, 2, n);
}

void decryptMsg(mpz_t r0, mpz_t arr2[], mpz_t r1, mpz_t r2, mpz_t r3, mpz_t r4)
{
    printf("decrypt msg func ----------------------------------------\n");

    int ascii_msg_len;
//    printf("ascii msg len: %d \n", ascii_msg_len);

    mpz_t arr[4];
    mpz_init_set(arr[0], r1);
    mpz_init_set(arr[1], r2);
    mpz_init_set(arr[2], r3);
    mpz_init_set(arr[3], r4);

    char *msg, lastBits1[7], lastBits2[7];

    for (int i=0; i<4; i++)
    {
        // μετατροπή του μηνύματος σε δυαδικό
        mpz_set(r0, arr[i]);

        gmp_printf("r0: %Zd\n", r0);

        ascii_msg_len = mpz_sizeinbase(r0, 2);

        printf("r0 msg len: %d \n", ascii_msg_len);
//        printf("str len msg: %d \n", strlen(msg));

        if (i == 0) msg = malloc(ascii_msg_len + 1);
        else if (strlen(msg) != ascii_msg_len) msg = realloc(msg, ascii_msg_len + 1);

        mpz_get_str(msg, 2, r0);
        printf("msg: %s \n", msg);

        strncpy(lastBits1, msg + ascii_msg_len - 6, 6);
        strncpy(lastBits2, msg + ascii_msg_len - 12, 6);

        lastBits1[6] = '\0';
        lastBits2[6] = '\0';

        printf("last bits 1: %s \n", lastBits1);
        printf("last bits 2: %s \n", lastBits2);

        if (strcmp(lastBits1, lastBits2) == 0)
        {
            msg = realloc(msg, ascii_msg_len - 5);
            msg[ascii_msg_len - 6] = '\0';
            printf("new msg: %s \n", msg);

            mpz_set_str(r0, msg, 2);
            gmp_printf("r0: %Zd\n", r0);

            break;
        }
    }
}

int main()
{
    srand(time(NULL));
    gmp_randinit_default(gmpRandState);

//    char *msg = "hello i am a noob";
//    char *msg = "hello my name is alex";
    char *msg = "hello my name is noob";
    printf("original msg: %s \n", msg);

    mpz_t n, p, q, c, ascii_msg, r0, r1, r2, r3, r4, arr[4];
    mpz_inits(n, p, q, c, ascii_msg, arr[0], arr[1], arr[2], arr[3], r0, r1, r2, r3, r4, NULL);


    gmp_randseed_ui(gmpRandState, rand());

    gen_keys(n, p, q, 512);

    convert_msg_to_int(ascii_msg, msg);

    encrytMsg(c, ascii_msg, n);

    roots(arr, r1, r2, r3, r4, n, p, q, c);

    decryptMsg(r0, arr, r1, r2, r3, r4);

    char *new_msg = convert_int_to_msg(r0);
    printf("new msg: %s \n", new_msg);

    mpz_clears(n, p, q, c, ascii_msg, arr[0], arr[1], arr[2], arr[3], r0, r1, r2, r3, r4, NULL);

    return 0;
}