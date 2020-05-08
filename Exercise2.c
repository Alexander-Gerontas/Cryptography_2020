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

    for (int i = 0; i < len; i++)
    {
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

void gen_keys(mpz_t p, mpz_t a, mpz_t a_d, mpz_t d, int bit)
{
    mpz_t rand_num, f_n, tmp;
    mpz_inits(rand_num, f_n, tmp, NULL);

    // Δημιουργία τυχαίου πρώτου p 200 bit.
    mpz_urandomb(rand_num, gmpRandState, bit);
    mpz_nextprime(p, rand_num);

    // Καθώς ο p είναι πρώτος η συνάρτηση φ(n) του Euler μας δίνει φ(p) = p - 1.
    mpz_sub_ui(f_n, p, 1);

    // Δημιουργούμε μια τυχαία μεταβλητή 100 bit.
    mpz_urandomb(tmp, gmpRandState, bit/2);

    // Η αναζήτηση του γεννήτορα ξεκινάει από το p/tmp. Δεν υπάρχει συγκεκριμένος λόγος, γίνεται απλά για τυχαιότητα.
    mpz_div(a, p, tmp);

    // Επανάληψη μέχρι να βρεθεί ενας γεννήτορας. Ο γεννήτορας πρέπει να έχει την ιδιότητα α^φ(n) ≡ 1 mod p.
    // Καθώς ο p είναι πρώτος δεν χρειάζεται να ελέγξουμε τον περιορισμό gcd(a,p) = 1.
    while (1)
    {
        mpz_add_ui(a, a, 1);

        mpz_powm(tmp, a, f_n, p);

        if (mpz_cmp_ui(tmp, 1) == 0)
            break;
    }

    // Δημιουργία τυχαίου d 6 bit και υπολογισμός του a^d.
    mpz_urandomb(d, gmpRandState, 6);
    mpz_pow_ui(a_d, a, mpz_get_ui(d));

    mpz_clears(rand_num, f_n, tmp, NULL);
}

void encryptMsg(mpz_t gamma, mpz_t delta, mpz_t p, mpz_t a, mpz_t a_d, char *plaintext)
{
    mpz_t tmp, k, ascii_plaintext;
    mpz_inits(tmp, k, ascii_plaintext, NULL);

    // Μετατροπή του plaintext σε ascii.
    convert_plaintext_to_ascii(ascii_plaintext, plaintext);
    gmp_printf("plaintext in ascii %Zd\n", ascii_plaintext);

    // Εάν το κρυπτοκείμενο σε ascii ξεπερνάει το p το πρόγραμμα τερματίζει.
    if (mpz_cmp(ascii_plaintext, p) >= 0)
    {
        gmp_printf("your message exceeds maximum message length: %Zd \n", p);
        exit(EXIT_FAILURE);
    }

    // Δημιουργία τυχαίου k 7 bit.
    mpz_urandomb(k, gmpRandState, 7);

    // γ = α^k mod p.
    mpz_powm(gamma, a, k, p);

    // tmp = α^dk
    mpz_pow_ui(tmp, a_d, mpz_get_ui(k));
    mpz_mul(tmp, tmp, ascii_plaintext);

    // δ = m*a^dk mod p
    mpz_mod(delta, tmp, p);

    mpz_clears(tmp, k, ascii_plaintext, NULL);
}

char *decryptMsg(mpz_t m2, mpz_t p, mpz_t d, mpz_t gamma, mpz_t delta)
{
    mpz_t tmp, g1;
    mpz_inits(tmp, g1, NULL);

    // tmp = p-1-d
    mpz_sub_ui(tmp, p, 1);
    mpz_sub(tmp, tmp, d);

    // g1 = γ^(p-1-d) mod p
    mpz_powm(g1, gamma, tmp, p);

    // m2 = (γ^-d) * δ mod p
    mpz_mul(tmp, g1, delta);
    mpz_mod(m2, tmp, p);

    gmp_printf("plaintext in ascii after decryption %Zd\n", m2);

    // Μετατροπή του κρυπτοκειμένου σε ascii στο αρχικό κρυπτοκείμενο.
    char *decrypted_plaintext = convert_ascii_to_plaintext(m2);

    mpz_clears(tmp, g1, NULL);

    return decrypted_plaintext;
}

int main()
{
    srand(time(NULL));
    gmp_randinit(gmpRandState, GMP_RAND_ALG_LC, 120);
    gmp_randseed_ui(gmpRandState, rand());

    mpz_t p, a, a_d, d, gamma, delta, ascii_msg, m2;
    mpz_inits(p, a, a_d, d, gamma, delta, ascii_msg, m2, NULL);

    // Εισαγωγή μηνύματος εδώ.
    char *plaintext = "Hello my name is Alice";
    printf("plaintext: %s \n", plaintext);

    // Δημιουργία δημόσιου και ιδιωτικού κλειδιού 200 bit.
    gen_keys(p, a, a_d, d, 200);

    printf("public key --------------------------------------------------------------------------------------------\n");
    gmp_printf("p: %Zd\n", p);
    gmp_printf("a: %Zd\n", a);
    gmp_printf("a^d: %Zd\n", a_d);

    printf("private key -------------------------------------------------------------------------------------------\n");
    gmp_printf("d: %Zd\n", d);

    printf("encryption --------------------------------------------------------------------------------------------\n");
    printf("plaintext: %s \n", plaintext);

    encryptMsg(gamma, delta, p, a, a_d, plaintext);

    printf("ciphertext --------------------------------------------------------------------------------------------\n");
    gmp_printf("gamma: %Zd\n", gamma);
    gmp_printf("delta: %Zd\n", delta);

    char *decrypted_plaintext = decryptMsg(m2, p, d, gamma, delta);
    printf("plaintext after decryption: %s \n", decrypted_plaintext);

    gmp_randclear(gmpRandState);
    mpz_clears(p, a, a_d, d, gamma, delta, ascii_msg, m2, NULL);

    return 0;
}