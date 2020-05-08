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

void gen_keys(mpz_t n, mpz_t p, mpz_t q, int bit)
{
    mpz_t rand_num, tmp1, tmp2;
    mpz_inits(rand_num, tmp1, tmp2, NULL);

    while (1)
    {
        // Δημιουργία ενός τυχαίου αριθμού όπου το μέγεθος του σε bit δίνεται με όρισμα.
        mpz_urandomb(rand_num, gmpRandState, bit);

        // Οι πρώτοι αριθμοί p,q είναι οι επόμενοι 2 πρώτοι μετά τον τυχαίο αριθμό.
        mpz_nextprime(p, rand_num);
        mpz_nextprime(q, p);

        // tmp1 = p mod 4, tmp2 = q mod 4
        mpz_mod_ui(tmp1, p, 4);
        mpz_mod_ui(tmp2, q, 4);

        // H επανάληψη σταματάει όταν βρεθούν p,q έτσι ώστε p ≡ 3 mod 4 και q ≡ 3 mod 4
        if (mpz_cmp(tmp1, tmp2) == 0 && mpz_cmp_ui(tmp2, 3) == 0) break;
    }

    // n = p * q
    mpz_mul(n, p, q);

    mpz_clears(rand_num, tmp1, tmp2, NULL);
}

void encryptMsg(mpz_t c, mpz_t n, char *plaintext)
{
    mpz_t ascii_plaintext, modified_ascii_plaintext;
    mpz_inits(ascii_plaintext, modified_ascii_plaintext, NULL);

    char *ascii_plaintext_binary, last_bits[7];

    // Μετατροπή του κειμένου σε μία ακολουθία ascii.
    convert_plaintext_to_ascii(ascii_plaintext, plaintext);
    gmp_printf("plaintext in ascii: \n%Zd \n", ascii_plaintext);

    int len = mpz_sizeinbase(ascii_plaintext, 2);

    // μετατροπή του μηνύματος σε δυαδικό.
    ascii_plaintext_binary = malloc(len + 7);
    mpz_get_str(ascii_plaintext_binary, 2, ascii_plaintext);

    printf("ascii plaintext in binary: \n%s \n", ascii_plaintext_binary);

    // Αντιγραφή των τελευταίων 6 bit του μηνύματος στην μεταβλητή last_bits
    strncpy(last_bits, ascii_plaintext_binary + len - 6, 6);

    // Προσθήκη των 6 bit στο τέλος του μηνύματος.
    strcat(ascii_plaintext_binary, last_bits);
    ascii_plaintext_binary[len + 6] = '\0';

    printf("ascii plaintext in binary with extra bits: \n%s \n", ascii_plaintext_binary);

    mpz_set_str(modified_ascii_plaintext, ascii_plaintext_binary, 2);

    gmp_printf("ascii plaintext in decimal with extra bits: \n%Zd \n", modified_ascii_plaintext);

    // Το πρόγραμμα τερματίζει εάν το μήνυμα ξεπερνάει το εύρος [0, n-1].
    if (mpz_cmp(modified_ascii_plaintext, n) >= 0)
    {
        gmp_printf("your message exceeds maximum message length: \n%Zd \n", n);
        exit(EXIT_FAILURE);
    }

    // Yπολογισμός της τιμής c = m^2 mod n
    mpz_powm_ui(c, modified_ascii_plaintext, 2, n);

    mpz_clears(ascii_plaintext, modified_ascii_plaintext, NULL);
}

void calculate_roots(mpz_t arr[], mpz_t n, mpz_t p, mpz_t q, mpz_t c)
{
    mpz_t g, a, b, p1, q1, r, s, tmp, tmp1, tmp2, r1, r2, r3, r4;
    mpz_inits(g, a, b, p1, q1, r, s, tmp, tmp1, tmp2, r1, r2, r3, r4, NULL);

    mpz_gcdext(g, a, b, p, q);

    // Δημιουργία p1, q1 όπου p1 = (p+1)/4, q1 = (q+1)/4
    mpz_add_ui(tmp1, p, 1);
    mpz_add_ui(tmp2, q, 1);

    mpz_div_ui(p1, tmp1, 4);
    mpz_div_ui(q1, tmp2, 4);

    // Υπολογισμός των τιμών r = c^[(p+1)/4] mod p, s = c^[(q+1)/4] mod q
    mpz_powm(r, c, p1, p);
    mpz_powm(s, c, q1, q);

    // tmp1 = aps
    mpz_mul(tmp, a, p);
    mpz_mul(tmp1, tmp, s);

    // tmp2 = bqr
    mpz_mul(tmp, b, q);
    mpz_mul(tmp2, tmp, r);

    // υπολογισμός 1ης ρίζας (r1): x = (aps + bqr) mod n
    mpz_add(tmp, tmp1, tmp2);
    mpz_mod(r1, tmp, n);

    // υπολογισμός 2ης ρίζας (r2): y = (aps - bqr) mod n
    mpz_sub(tmp, tmp1, tmp2);
    mpz_mod(r2, tmp, n);

    // υπολογισμός 3ης ρίζας (r3): -x mod n
    mpz_neg(tmp, r1);
    mpz_mod(r3, tmp, n);

    // υπολογισμός 4ης ρίζας (r4): -y mod n
    mpz_neg(tmp, r2);
    mpz_mod(r4, tmp, n);

    // Αποθήκευση των ριζών στον πίνακα arr.
    mpz_set(arr[0], r1);
    mpz_set(arr[1], r2);
    mpz_set(arr[2], r3);
    mpz_set(arr[3], r4);

    mpz_clears(g, a, b, p1, q1, r, s, tmp, tmp1, tmp2, r1, r2, r3, r4, NULL);
}

void decryptMsg(mpz_t r0, mpz_t *arr)
{
    int root_len_in_bin;
    char *root, *ascii_plaintext, lastBits1[7], lastBits2[7];

    for (int i=0; i<4; i++)
    {
        // Αποθήκευση μιας ρίζας από τον πίνακα arr[] στην μεταβλητή r0.
        mpz_set(r0, arr[i]);

        // Mετατροπή της ρίζας σε δυαδικό.
        root_len_in_bin = mpz_sizeinbase(r0, 2);

        root = malloc(root_len_in_bin + 1);

        mpz_get_str(root, 2, r0);

        // Αποθήκευση των τελευταίων 12 bit, στις μεταβλητές lastBits1, lastBits2.
        strncpy(lastBits1, root + root_len_in_bin - 6, 6);
        strncpy(lastBits2, root + root_len_in_bin - 12, 6);

        lastBits1[6] = '\0';
        lastBits2[6] = '\0';

        // Σύγκριση των μεταβλητών lastBits1, lastBits2.
        if (strcmp(lastBits1, lastBits2) == 0)
        {
            printf("last 6 bits of root %d are replicated\n", i);
            printf("root in binary: \n%s \n", root);
            printf("last bits 1: %s \n", lastBits1);
            printf("last bits 2: %s \n", lastBits2);

            ascii_plaintext = malloc(root_len_in_bin - 5);

            // Αποκοπή των τελευταίων 6 bit που επαναλαμβάνονται.
            strncpy(ascii_plaintext, root, root_len_in_bin - 6);

            mpz_set_str(r0, ascii_plaintext, 2);
            gmp_printf("decrypted plaintext in ascii: \n%Zd \n", r0);

            break;
        } else free(root);
    }
}

int main()
{
    srand(time(NULL));
    gmp_randinit(gmpRandState, GMP_RAND_ALG_LC, 120);
    gmp_randseed_ui(gmpRandState, rand());

    mpz_t n, p, q, c, ascii_msg, r0, arr[4];
    mpz_inits(n, p, q, c, ascii_msg, r0, arr[0], arr[1], arr[2], arr[3], NULL);

    char *plaintext = "Hello my name is Alice\nMy best friend is Bob";

    gen_keys(n, p, q, 200);

    printf("public key --------------------------------------------------------------------------------------------\n");
    gmp_printf("n: %Zd\n", n);

    printf("private key -------------------------------------------------------------------------------------------\n");
    gmp_printf("p: %Zd\n", p);
    gmp_printf("q: %Zd\n\n", q);

    printf("encryption --------------------------------------------------------------------------------------------\n");
    printf("plaintext: \n%s \n", plaintext);

    encryptMsg(c, n, plaintext);
    gmp_printf("ciphertext: \n%Zd\n\n", c);

    calculate_roots(arr, n, p, q, c);

    printf("decryption --------------------------------------------------------------------------------------------\n");
    decryptMsg(r0, arr);

    char *decrypted_plaintext = convert_ascii_to_plaintext(r0);
    printf("plaintext after decryption: %s \n", decrypted_plaintext);

    gmp_randclear(gmpRandState);
    mpz_clears(n, p, q, c, ascii_msg, r0, arr[0], arr[1], arr[2], arr[3], NULL);

    return 0;
}
