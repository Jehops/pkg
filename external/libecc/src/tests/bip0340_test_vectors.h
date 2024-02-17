/*
 *  Copyright (C) 2022 - This file is part of libecc project
 *
 *  Authors:
 *      Arnaud EBALARD <arnaud.ebalard@ssi.gouv.fr>
 *      Ryad BENADJILA <ryadbenadjila@gmail.com>
 *
 *  This software is licensed under a dual BSD and GPL v2 license.
 *  See LICENSE file at the root folder of the project.
 */
#ifndef __BIP0340_TEST_VECTORS_H__
#define __BIP0340_TEST_VECTORS_H__

#if defined(WITH_HASH_SHA256) && defined(WITH_CURVE_SECP256K1)
/************************************************/
static const u8 bip0340_1_test_vectors_priv_key[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03,
 };
static const u8 bip0340_1_test_vectors_expected_sig[] = {
	0xE9, 0x07, 0x83, 0x1F, 0x80, 0x84, 0x8D, 0x10, 0x69, 0xA5, 0x37, 0x1B, 0x40, 0x24, 0x10, 0x36, 0x4B, 0xDF, 0x1C, 0x5F, 0x83, 0x07, 0xB0, 0x08, 0x4C, 0x55, 0xF1, 0xCE, 0x2D, 0xCA, 0x82, 0x15, 0x25, 0xF6, 0x6A, 0x4A, 0x85, 0xEA, 0x8B, 0x71, 0xE4, 0x82, 0xA7, 0x4F, 0x38, 0x2D, 0x2C, 0xE5, 0xEB, 0xEE, 0xE8, 0xFD, 0xB2, 0x17, 0x2F, 0x47, 0x7D, 0xF4, 0x90, 0x0D, 0x31, 0x05, 0x36, 0xC0,
 };
static int bip0340_1_nn_random_test_vector(nn_t out, nn_src_t q)
{
        int ret, cmp;

        /*
         * Fixed auxiliary random for BIP0340
         * Test vectors from:
	 * https://github.com/bitcoin/bips/blob/master/bip-0340/test-vectors.csv
         */
        const u8 k_buf[] = {
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        };

        ret = nn_init_from_buf(out, k_buf, sizeof(k_buf)); EG(ret, err);
        ret = nn_cmp(out, q, &cmp); EG(ret, err);

        ret = (cmp >= 0) ? -1 : 0;

err:
        return ret;
}

static const ec_test_case bip0340_1_test_case = {
        .name = "BIP0340-SHA256/secp256k1 1",
        .ec_str_p = &secp256k1_str_params,
        .priv_key = bip0340_1_test_vectors_priv_key,
        .priv_key_len = sizeof(bip0340_1_test_vectors_priv_key),
        .nn_random = bip0340_1_nn_random_test_vector,
        .hash_type = SHA256,
        .msg = "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
        .msglen = 32,
        .sig_type = BIP0340,
        .exp_sig = bip0340_1_test_vectors_expected_sig,
        .exp_siglen = sizeof(bip0340_1_test_vectors_expected_sig),
	.adata = NULL,
	.adata_len = 0
};

/************************************************/
static const u8 bip0340_2_test_vectors_priv_key[] = {
	0xB7, 0xE1, 0x51, 0x62, 0x8A, 0xED, 0x2A, 0x6A, 0xBF, 0x71, 0x58, 0x80, 0x9C, 0xF4, 0xF3, 0xC7, 0x62, 0xE7, 0x16, 0x0F, 0x38, 0xB4, 0xDA, 0x56, 0xA7, 0x84, 0xD9, 0x04, 0x51, 0x90, 0xCF, 0xEF,
 };
static const u8 bip0340_2_test_vectors_expected_sig[] = {
	0x68, 0x96, 0xBD, 0x60, 0xEE, 0xAE, 0x29, 0x6D, 0xB4, 0x8A, 0x22, 0x9F, 0xF7, 0x1D, 0xFE, 0x07, 0x1B, 0xDE, 0x41, 0x3E, 0x6D, 0x43, 0xF9, 0x17, 0xDC, 0x8D, 0xCF, 0x8C, 0x78, 0xDE, 0x33, 0x41, 0x89, 0x06, 0xD1, 0x1A, 0xC9, 0x76, 0xAB, 0xCC, 0xB2, 0x0B, 0x09, 0x12, 0x92, 0xBF, 0xF4, 0xEA, 0x89, 0x7E, 0xFC, 0xB6, 0x39, 0xEA, 0x87, 0x1C, 0xFA, 0x95, 0xF6, 0xDE, 0x33, 0x9E, 0x4B, 0x0A,
 };
static int bip0340_2_nn_random_test_vector(nn_t out, nn_src_t q)
{
        int ret, cmp;

        /*
         * Fixed auxiliary random for BIP0340
         * Test vectors from:
	 * https://github.com/bitcoin/bips/blob/master/bip-0340/test-vectors.csv
         */
        const u8 k_buf[] = {
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
        };

        ret = nn_init_from_buf(out, k_buf, sizeof(k_buf)); EG(ret, err);
        ret = nn_cmp(out, q, &cmp); EG(ret, err);

        ret = (cmp >= 0) ? -1 : 0;

err:
        return ret;
}

static const ec_test_case bip0340_2_test_case = {
        .name = "BIP0340-SHA256/secp256k1 2",
        .ec_str_p = &secp256k1_str_params,
        .priv_key = bip0340_2_test_vectors_priv_key,
        .priv_key_len = sizeof(bip0340_2_test_vectors_priv_key),
        .nn_random = bip0340_2_nn_random_test_vector,
        .hash_type = SHA256,
        .msg = "\x24\x3F\x6A\x88\x85\xA3\x08\xD3\x13\x19\x8A\x2E\x03\x70\x73\x44\xA4\x09\x38\x22\x29\x9F\x31\xD0\x08\x2E\xFA\x98\xEC\x4E\x6C\x89",
        .msglen = 32,
        .sig_type = BIP0340,
        .exp_sig = bip0340_2_test_vectors_expected_sig,
        .exp_siglen = sizeof(bip0340_2_test_vectors_expected_sig),
	.adata = NULL,
	.adata_len = 0
};

/************************************************/
static const u8 bip0340_3_test_vectors_priv_key[] = {
	0xC9, 0x0F, 0xDA, 0xA2, 0x21, 0x68, 0xC2, 0x34, 0xC4, 0xC6, 0x62, 0x8B, 0x80, 0xDC, 0x1C, 0xD1, 0x29, 0x02, 0x4E, 0x08, 0x8A, 0x67, 0xCC, 0x74, 0x02, 0x0B, 0xBE, 0xA6, 0x3B, 0x14, 0xE5, 0xC9,
 };
static const u8 bip0340_3_test_vectors_expected_sig[] = {
	0x58, 0x31, 0xAA, 0xEE, 0xD7, 0xB4, 0x4B, 0xB7, 0x4E, 0x5E, 0xAB, 0x94, 0xBA, 0x9D, 0x42, 0x94, 0xC4, 0x9B, 0xCF, 0x2A, 0x60, 0x72, 0x8D, 0x8B, 0x4C, 0x20, 0x0F, 0x50, 0xDD, 0x31, 0x3C, 0x1B, 0xAB, 0x74, 0x58, 0x79, 0xA5, 0xAD, 0x95, 0x4A, 0x72, 0xC4, 0x5A, 0x91, 0xC3, 0xA5, 0x1D, 0x3C, 0x7A, 0xDE, 0xA9, 0x8D, 0x82, 0xF8, 0x48, 0x1E, 0x0E, 0x1E, 0x03, 0x67, 0x4A, 0x6F, 0x3F, 0xB7,
 };
static int bip0340_3_nn_random_test_vector(nn_t out, nn_src_t q)
{
        int ret, cmp;

        /*
         * Fixed auxiliary random for BIP0340
         * Test vectors from:
	 * https://github.com/bitcoin/bips/blob/master/bip-0340/test-vectors.csv
         */
        const u8 k_buf[] = {
		0xC8, 0x7A, 0xA5, 0x38, 0x24, 0xB4, 0xD7, 0xAE, 0x2E, 0xB0, 0x35, 0xA2, 0xB5, 0xBB, 0xBC, 0xCC, 0x08, 0x0E, 0x76, 0xCD, 0xC6, 0xD1, 0x69, 0x2C, 0x4B, 0x0B, 0x62, 0xD7, 0x98, 0xE6, 0xD9, 0x06,
        };

        ret = nn_init_from_buf(out, k_buf, sizeof(k_buf)); EG(ret, err);
        ret = nn_cmp(out, q, &cmp); EG(ret, err);

        ret = (cmp >= 0) ? -1 : 0;

err:
        return ret;
}

static const ec_test_case bip0340_3_test_case = {
        .name = "BIP0340-SHA256/secp256k1 3",
        .ec_str_p = &secp256k1_str_params,
        .priv_key = bip0340_3_test_vectors_priv_key,
        .priv_key_len = sizeof(bip0340_3_test_vectors_priv_key),
        .nn_random = bip0340_3_nn_random_test_vector,
        .hash_type = SHA256,
        .msg = "\x7E\x2D\x58\xD8\xB3\xBC\xDF\x1A\xBA\xDE\xC7\x82\x90\x54\xF9\x0D\xDA\x98\x05\xAA\xB5\x6C\x77\x33\x30\x24\xB9\xD0\xA5\x08\xB7\x5C",
        .msglen = 32,
        .sig_type = BIP0340,
        .exp_sig = bip0340_3_test_vectors_expected_sig,
        .exp_siglen = sizeof(bip0340_3_test_vectors_expected_sig),
	.adata = NULL,
	.adata_len = 0
};

/************************************************/
static const u8 bip0340_4_test_vectors_priv_key[] = {
	0x0B, 0x43, 0x2B, 0x26, 0x77, 0x93, 0x73, 0x81, 0xAE, 0xF0, 0x5B, 0xB0, 0x2A, 0x66, 0xEC, 0xD0, 0x12, 0x77, 0x30, 0x62, 0xCF, 0x3F, 0xA2, 0x54, 0x9E, 0x44, 0xF5, 0x8E, 0xD2, 0x40, 0x17, 0x10,
 };
static const u8 bip0340_4_test_vectors_expected_sig[] = {
	0x7E, 0xB0, 0x50, 0x97, 0x57, 0xE2, 0x46, 0xF1, 0x94, 0x49, 0x88, 0x56, 0x51, 0x61, 0x1C, 0xB9, 0x65, 0xEC, 0xC1, 0xA1, 0x87, 0xDD, 0x51, 0xB6, 0x4F, 0xDA, 0x1E, 0xDC, 0x96, 0x37, 0xD5, 0xEC, 0x97, 0x58, 0x2B, 0x9C, 0xB1, 0x3D, 0xB3, 0x93, 0x37, 0x05, 0xB3, 0x2B, 0xA9, 0x82, 0xAF, 0x5A, 0xF2, 0x5F, 0xD7, 0x88, 0x81, 0xEB, 0xB3, 0x27, 0x71, 0xFC, 0x59, 0x22, 0xEF, 0xC6, 0x6E, 0xA3,
 };
static int bip0340_4_nn_random_test_vector(nn_t out, nn_src_t q)
{
        int ret, cmp;

        /*
         * Fixed auxiliary random for BIP0340
         * Test vectors from:
	 * https://github.com/bitcoin/bips/blob/master/bip-0340/test-vectors.csv
         */
        const u8 k_buf[] = {
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        };

        ret = nn_init_from_buf(out, k_buf, sizeof(k_buf)); EG(ret, err);
        ret = nn_cmp(out, q, &cmp); EG(ret, err);

        ret = (cmp >= 0) ? -1 : 0;

err:
        return ret;
}

static const ec_test_case bip0340_4_test_case = {
        .name = "BIP0340-SHA256/secp256k1 4",
        .ec_str_p = &secp256k1_str_params,
        .priv_key = bip0340_4_test_vectors_priv_key,
        .priv_key_len = sizeof(bip0340_4_test_vectors_priv_key),
        .nn_random = bip0340_4_nn_random_test_vector,
        .hash_type = SHA256,
        .msg = "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF",
        .msglen = 32,
        .sig_type = BIP0340,
        .exp_sig = bip0340_4_test_vectors_expected_sig,
        .exp_siglen = sizeof(bip0340_4_test_vectors_expected_sig),
	.adata = NULL,
	.adata_len = 0
};

#endif

/************************************************/
#define BIP0340_ALL_TESTS() \
        &bip0340_1_test_case, \
        &bip0340_2_test_case, \
        &bip0340_3_test_case, \
        &bip0340_4_test_case,

#endif /* __BIP0340_TEST_VECTORS_H__ */
