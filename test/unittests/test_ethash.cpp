// Copyright 2018 Pawel Bylica.
// Licensed under the Apache License, Version 2.0. See the LICENSE file.

#pragma GCC diagnostic ignored "-Wpedantic"
#pragma clang diagnostic ignored "-Wpedantic"

#include <ethash/endianness.hpp>
#include <ethash/ethash-internal.hpp>
#include <ethash/ethash.hpp>
#include <ethash/keccak.hpp>
#include <ethash/params.hpp>

#include "helpers.hpp"
#include "test_cases.hpp"

#include <gtest/gtest.h>

#include <array>
#include <future>

using namespace ethash;

namespace
{
/// Creates the epoch context of the correct size but filled with fake data.
epoch_context create_epoch_context_mock(int epoch_number)
{
    // Prepare a constant endianness-independent cache item.
    hash512 fill;
    std::fill_n(fill.words, sizeof(hash512) / sizeof(uint64_t), fix_endianness(0xe14a54a1b2c3d4e5));
    const size_t cache_size = calculate_light_cache_size(epoch_number);

    epoch_context context = {};
    context.cache = light_cache(cache_size, fill);
    context.full_dataset_size = calculate_full_dataset_size(epoch_number);
    return context;
}
}


TEST(hash, hash256_from_bytes)
{
    const uint8_t bytes[32] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
        20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31};
    auto h = hash256::from_bytes(bytes);
    EXPECT_EQ(to_hex(h), "000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f");
}

TEST(hash, hash1024_init)
{
    ethash::hash1024 hash;
    for (auto w : hash.words)
        EXPECT_EQ(w, 0);
}

struct dataset_size_test_case
{
    int epoch_number;
    uint64_t light_cache_size;
    uint64_t full_dataset_size;
};

/// Test cases for dataset sizes are random picked from generated ethash sizes.
/// See https://github.com/ethereum/wiki/wiki/Ethash#data-sizes.
static dataset_size_test_case dataset_size_test_cases[] = {
    {0, 16776896, 1073739904},
    {14, 18611392, 1191180416},
    {17, 19004224, 1216345216},
    {56, 24116672, 1543503488},
    {158, 37486528, 2399139968},
    {203, 43382848, 2776625536},
    {211, 44433344, 2843734144},
    {272, 52427968, 3355440512},
    {350, 62651584, 4009751168},
    {412, 70778816, 4529846144},
    {464, 77593664, 4966054784},
    {530, 86244416, 5519703424},
    {656, 102760384, 6576662912},
    {657, 102890432, 6585055616},
    {658, 103021888, 6593443456},
    {739, 113639104, 7272921472},
    {751, 115212224, 7373585792},
    {798, 121372352, 7767849088},
    {810, 122945344, 7868512384},
    {862, 129760832, 8304715136},
    {882, 132382528, 8472492928},
    {920, 137363392, 8791260032},
    {977, 144832448, 9269411456},
    {1093, 160038464, 10242489472},
    {1096, 160430656, 10267656064},
    {1119, 163446592, 10460589952},
    {1125, 164233024, 10510923392},
    {1165, 169475648, 10846469248},
    {1168, 169866304, 10871631488},
    {1174, 170655424, 10921964672},
    {1211, 175504832, 11232345728},
    {1244, 179830208, 11509169024},
    {1410, 201588544, 12901672832},
    {1418, 202636352, 12968786816},
    {1436, 204996544, 13119782528},
    {1502, 213645632, 13673430656},
    {1512, 214956992, 13757316224},
    {1535, 217972672, 13950253696},
    {1538, 218364736, 13975417216},
    {1554, 220461632, 14109635968},
    {1571, 222690368, 14252243072},
    {1602, 226754368, 14512291712},
    {1621, 229243328, 14671675264},
    {1630, 230424512, 14747172736},
    {1698, 239335232, 15317596544},
    {1746, 245628736, 15720251264},
    {1790, 251395136, 16089346688},
    {1818, 255065792, 16324230016},
    {1912, 267386432, 17112759424},
    {1928, 269482688, 17246976896},
    {1956, 273153856, 17481857408},
    {2047, 285081536, 18245220736},
    {30000, 3948936512, 252731976832},
    {32639, 4294836032, 274869514624},
};

TEST(ethash, light_cache_size)
{
    for (const auto& t : dataset_size_test_cases)
    {
        uint64_t size = ethash::calculate_light_cache_size(t.epoch_number);
        EXPECT_EQ(size, t.light_cache_size) << "epoch: " << t.epoch_number;
    }
}

TEST(ethash, full_dataset_size)
{
    for (const auto& t : dataset_size_test_cases)
    {
        uint64_t size = ethash::calculate_full_dataset_size(t.epoch_number);
        EXPECT_EQ(size, t.full_dataset_size) << "epoch: " << t.epoch_number;
    }
}


TEST(ethash, calculate_seed_0)
{
    hash256 s0 = calculate_seed(0);
    for (auto w : s0.words)
        EXPECT_EQ(w, 0);
}

TEST(ethash, calculate_seed_1)
{
    hash256 seed = calculate_seed(1);
    std::string sh = to_hex(seed);
    EXPECT_EQ(sh, "290decd9548b62a8d60345a988386fc84ba6bc95484008f6362f93160ef3e563");
}

TEST(ethash, calculate_seed_171)
{
    hash256 seed = calculate_seed(171);
    std::string sh = to_hex(seed);
    EXPECT_EQ(sh, "a9b0e0c9aca72c07ba06b5bbdae8b8f69e61878301508473379bb4f71807d707");
}

TEST(ethash, calculate_seed_2048)
{
    hash256 seed = calculate_seed(2048);
    std::string sh = to_hex(seed);
    EXPECT_EQ(sh, "20a7678ca7b50829183baac2e1e3c43fa3c4bcbc171b11cf5a9f30bebd172920");
}

TEST(ethash, calculate_seed_29998)
{
    hash256 seed = calculate_seed(29998);
    std::string sh = to_hex(seed);
    EXPECT_EQ(sh, "1222b1faed7f93098f8ae498621fb3479805a664b70186063861c46596c66164");
}

TEST(ethash, calculate_seed_29999)
{
    hash256 seed = calculate_seed(29999);
    std::string sh = to_hex(seed);
    EXPECT_EQ(sh, "ee1d0f61b054dff0f3025ebba821d405c8dc19a983e582e9fa5436fc3e7a07d8");
}

TEST(ethash, find_epoch_number_29998)
{
    hash256 seed = to_hash256("1222b1faed7f93098f8ae498621fb3479805a664b70186063861c46596c66164");

    int epoch = find_epoch_number(seed);
    EXPECT_EQ(epoch, 29998);
}

TEST(ethash, find_epoch_number_double_ascending)
{
    hash256 seed_29998 =
        to_hash256("1222b1faed7f93098f8ae498621fb3479805a664b70186063861c46596c66164");
    hash256 seed_29999 =
        to_hash256("ee1d0f61b054dff0f3025ebba821d405c8dc19a983e582e9fa5436fc3e7a07d8");

    int epoch = find_epoch_number(seed_29998);
    EXPECT_EQ(epoch, 29998);

    // The second call should be fast.
    epoch = find_epoch_number(seed_29999);
    EXPECT_EQ(epoch, 29999);
}

TEST(ethash, find_epoch_number_double_descending)
{
    hash256 seed_29998 =
        to_hash256("1222b1faed7f93098f8ae498621fb3479805a664b70186063861c46596c66164");
    hash256 seed_29999 =
        to_hash256("ee1d0f61b054dff0f3025ebba821d405c8dc19a983e582e9fa5436fc3e7a07d8");

    int epoch = find_epoch_number(seed_29999);
    EXPECT_EQ(epoch, 29999);

    // The second call should be fast.
    epoch = find_epoch_number(seed_29998);
    EXPECT_EQ(epoch, 29998);
}

TEST(ethash, find_epoch_number_sequential)
{
    hash256 seed;
    for (int i = 0; i < 30000; ++i)
    {
        auto e = find_epoch_number(seed);
        EXPECT_EQ(e, i);
        seed = keccak256(seed);
    }
}

TEST(ethash, find_epoch_number_sequential_gap)
{
    constexpr int start_epoch = 200;
    hash256 seed = calculate_seed(start_epoch);
    for (int i = start_epoch; i < 30000; ++i)
    {
        auto e = find_epoch_number(seed);
        EXPECT_EQ(e, i);
        seed = keccak256(seed);
    }
}

TEST(ethash, find_epoch_number_descending)
{
    for (int i = 2050; i >= 0; --i)
    {
        auto seed = calculate_seed(i);
        auto e = find_epoch_number(seed);
        EXPECT_EQ(e, i);
    }
}

TEST(ethash, find_epoch_number_invalid)
{
    hash256 fake_seed;
    fake_seed.hwords[0] = 1;
    int epoch = find_epoch_number(fake_seed);
    EXPECT_EQ(epoch, -1);
}

TEST(ethash, find_epoch_number_epoch_too_high)
{
    hash256 seed = calculate_seed(30000);
    int epoch = find_epoch_number(seed);
    EXPECT_EQ(epoch, -1);
}

TEST(ethash_multithreaded, find_epoch_number_sequential)
{
    auto fn = [] {
        hash256 seed;
        for (int i = 0; i < 30000; ++i)
        {
            auto e = find_epoch_number(seed);
            EXPECT_EQ(e, i);
            seed = keccak256(seed);
        }
    };

    std::array<std::future<void>, 4> futures;
    for (auto& f : futures)
        f = std::async(std::launch::async, fn);
    for (auto& f : futures)
        f.wait();
}

TEST(ethash, version)
{
    std::string expected_version{"0.1.0.dev0"};
    EXPECT_EQ(version(), expected_version);
}

TEST(ethash, get_epoch_number)
{
    EXPECT_EQ(get_epoch_number(0), 0);
    EXPECT_EQ(get_epoch_number(1), 0);
    EXPECT_EQ(get_epoch_number(29999), 0);
    EXPECT_EQ(get_epoch_number(30000), 1);
    EXPECT_EQ(get_epoch_number(30001), 1);
    EXPECT_EQ(get_epoch_number(30002), 1);
    EXPECT_EQ(get_epoch_number(5000000), 166);
}

TEST(ethash, light_cache)
{
    struct light_cache_test_case
    {
        struct item_test
        {
            size_t index;
            const char* hash_hex;
        };

        int epoch_number;
        std::vector<item_test> item_tests;
    };

    // clang-format off
    light_cache_test_case test_cases[] = {
        {0,
            {
                {0,      "5e493e76a1318e50815c6ce77950425532964ebbb8dcf94718991fa9a82eaf37658de68ca6fe078884e803da3a26a4aa56420a6867ebcd9ab0f29b08d1c48fed"},
                {1,      "47bcbf5825d4ba95ed8d9074291fa2242cd6aabab201fd923565086c6d19b93c6936bba8def3c8e40f891288f9926eb87318c30b22611653439801d2a596a78c"},
                {52459,  "a1cfe59d6fcb93fe54a090f4d440472253b1d18962acc64a949c955ee5785c0a577bbbe8e02acdcc2d829df28357ec6ba7bb37d5026a6bd2c43e2f990b416fa9"},
                {176162, "b67128c2b592f834aaa17dad7990596f62cb85b11b3ee7072fc9f8d054366d30bed498a1d2e296920ce01ec0e32559c434a6a20329e23020ecce615ff7938c23"},
                {194590, "05ed082c5cb2c511aceb8ecd2dc785b46f77122e788e3dacbc76ecee5050b83be076758ca9aa6528e384edbf7c3ccb96ca0622fcbdd643380bd9d4dd2e293a59"},
                {262137, "4615b9c6e721f9c7b36ab317e8f125cc4edc42fabf391535da5ef15090182c2523a007d04aeca6fd1d3f926f5ae27a65400404fcdc80dd7da1d46fdaac030d06"},
                {262138, "724f2f86c24c487809dc3897acbbd32d5d791e4536aa1520e65e93891a40dde5887899ffc556cbd174f426e32ae2ab711be859601c024d1514b29a27370b662e"},
            }},
        {171,
            {
                {0,      "bb7f72d3813a9a9ad14e6396a3be9edba40197f8c6b60a26ca188260227f5d287616e9c93da7de35fd237c191c36cdcc00abd98dfcacd11d1f2544aa52000917"},
                {1,      "460036cc618bec3abd406c0cb53182c4b4dbbcc0266995d0f7dd3511b5a4d599405cb6cc3aff5874a7cfb87e71688ba0bb63c4aa756fd36676aa947d53c9a1b1"},
                {173473, "d5cd1e7ced40ad920d977ff3a28e11d8b42b556c44e3cf48e4cce0a0854a188182f971db468756f951ed9b76c9ecf3dbbdd0209e89febe893a88785e3c714e37"},
                {202503, "609227c6156334b0633526c032ab68d82414aee92461e60e75c5751e09e5c4d645e13e1332d2cddd993da5b5b872b18b5c26eabef39acf4fc610120685aa4b24"},
                {227478, "fb5bd4942d7da4c80dc4075e5dc121a1fb1d42a7feae844d5fe321cda1db3dbdd4035dadc5b69585f77580a61260dc36a75549995f6bcba5539da8fa726c1dd5"},
                {612347, "7a3373b5c50b8950de5172b0f2b7565ecaf00e3543de972e21122fb31505085b196c6be11738d6fce828dac744159bbd62381beddfcbd00586b8a84c6c4468c8"},
                {612348, "ec45073bd7820fe58ea29fa89375050cfb1da7bdb17b79f20f8e427bef1cdc0976d1291597fece7f538e5281a9d8df3f0b842bb691ade89d3864dfa965c7e187"},
            }},
        {2047,
            {
                {0,       "e887f74775c5ac3f2ed928d74dde3d8b821e9b9f251f6bb66ccc817c3c52e01b319d5cf394b0af867c871acca3375c0118ffbdafe4d7b7d23d7361cf14ed1458"},
                {1,       "9e6132d61fab636a6b95207c9d4437beabaa683d50a03a49e0fc79c1d375a067f2ced6d3be3f26e7e864a156035ad80d1a5295bf5e2be71722ed8f2708c50997"},
                {574988,  "d5f763c03e8343d4e9e55fffb45045bb25d3f9d50175453bec652e469f29419058e33bcddf8a39ba2cc84a8bf52ed58d48bcbe052b41827ed3c14b281bf088af"},
                {2778440, "d1eafd2b51d3c4485d1959ed49434c5e0684bd14e58e8b77244fca33939f64f396d205df1fb19b342f9b8605e4098ae6244d06a0a564cdd150f8e3a06681b5fb"},
                {3292172, "004aa90c2ffae1ac6c5573222a214c05a22c5d4d4db35747a945d7903aa1867ae57783cafd350fdaf23441da020403810213c3724be37e9353909d9a5283819d"},
                {4454397, "5a74ae8affebf2df925265b8467ae3f71b61814636d34bee677eeb308f28c7c18a0e6394c2c20b3026ac33386c0a2af38f92a1cc1633ea66ceefbf1e9090da57"},
                {4454398, "0504239bf95d918c1f09b9f05659376a4dd461f3b5025ab05fefd1fb6c41f08da90e38dcff6861bfe28ab3e09997b9f373f017c13f30818670c92267d5b91d55"},
            }},
    };
    // clang-format on


    for (const auto& t : test_cases)
    {
        auto* context = create_epoch_context(t.epoch_number);

        for (auto& u : t.item_tests)
        {
            ASSERT_LT(u.index, context->cache.size());
            EXPECT_EQ(to_hex(context->cache[u.index]), u.hash_hex)
                << "epoch: " << t.epoch_number << " item: " << u.index;
        }

        destroy_epoch_context(context);
    }
}

TEST(ethash, fake_dataset_partial_items)
{
    struct full_dataset_item_test_case
    {
        size_t index;
        const char* hash_hex;
    };

    // clang-format off
    full_dataset_item_test_case test_cases[] = {
        {0,          "cc40bef17615acac264c8fedc4df8ca02b949f8cef3e3d08b6711acb8a5254941249cdf50be5fccf0082e587f5520546db85250d853e98ee01fedb87252222c5"},
        {1,          "03489165b412bfe8085f73902383ae2fff5aa2882835f5f4dc68826f3c52172b98dfb0a94bacde60634d5aaa0b89ef2e7ac0fe024d4bf2ef9d3ff2569aeb1aa3"},
        {13,         "f90b200ec5dc92625fca180f97667aff09f93194b7c5f17fcb08ae7fc97198f24dd6f2c8d72ac76ffe435f506368978a8a784a4d89cad374ec58ce13a0e67689"},
        {171,        "0e538e98a6a0c2c114560777cfb48814bb8b5c314ed03d0c60367588daea53c6d87184f72fbc951c57305b67d766f2361220a6a4907018bd3bc70f69f8bcf87c"},
        {571,        "1cb34c582cc4a2ea96473349670703675ac6e4a6cfd5777ad5f496df1467456635227532821d5e07fb288a30180c5c483457ca57ffb3b1f3ddcbc4113bcc38cb"},
        {620,        "05c230cf73242696b4810ff5ddd522d07b63c83e54eb0da17b691f79db5247c9bac577b8af56462d86bbbff7a767634483b697c93d98c4950e2a856ef673b6fa"},
        {514079,     "949b9be7de5b3c009f5d8185b511bd3ddf29dfbf453905c91083cbfc01098455ad73d41a3b4af742509e3ae6cad96024b8ac5337a9c2f1a418c3875537b390a4"},
        {852881,     "e67f0331c7bd43d215988c862bfac14424e0d56fd6603cc281691f5ae229b6652b0adb311dc5dbdc124d66f90e699c16ed1c0f04efa03f9fd14888c42d841585"},
        {6133492,    "94b4c2bb240f039943d5612243be303d87c523ee2c319cd094960671716403dd2735cd7305a13d4b85d12e75997b2f6dfeaf77fcee0dd3e2c7b72135ca585bc9"},
        {6133493,    "aed4fd3f4d641fb3ad4cc5d8602a6c4b7b40ee2ca6cb694a679662992c744603e97a7dfc1b6ed94820556d9a7bfca3afb91175ab55837cad31a508dd8084100d"},
        {211322105,  "51c99716a2193f38f42c49c96e915db7a304c1d0ee9458e207d221c4ec287fc01a692d99433ea5fa99b3c9a71d899bb4bc88bcddbb9da1c1f3cbb62738c0a4b0"},
        {740620450,  "aa6a733cff55c9d1cc9058a1cd919076fe06cc9dea4f2db9aa17414dc60cc80087637f8536894335532b019da8066ac28ab76cfcd9236924e19328c777fa11be"},
        {4294967295, "8e4fdb5dc602598f10a42b5061132eec05299380db872a3caf04aa21e3d4970350394dfbd58c5ab54571b1be0cc9001d788c6b14cbf003d7decc2aaef1232b8c"},
    };
    // clang-format on

    // Mock the epoch context.
    epoch_context context = create_epoch_context_mock(0);

    for (const auto& t : test_cases)
    {
        const hash512 item = calculate_dataset_item_partial(context.cache, t.index);
        EXPECT_EQ(to_hex(item), t.hash_hex) << "index: " << t.index;
    }
}

TEST(ethash, fake_dataset_items)
{
    struct full_dataset_item_test_case
    {
        size_t index;
        const char* hash1_hex;
        const char* hash2_hex;
    };

    // clang-format off
    full_dataset_item_test_case test_cases[] = {
        {0,
            "cc40bef17615acac264c8fedc4df8ca02b949f8cef3e3d08b6711acb8a5254941249cdf50be5fccf0082e587f5520546db85250d853e98ee01fedb87252222c5",
            "03489165b412bfe8085f73902383ae2fff5aa2882835f5f4dc68826f3c52172b98dfb0a94bacde60634d5aaa0b89ef2e7ac0fe024d4bf2ef9d3ff2569aeb1aa3"},
        {1,
            "9d05d44ed96bcaef3c94806b360a828ae04e78208cd496ffeaa72bc2ffd0932daf92ffae790030474e8382a5d527c97696edafa58536b0ab86f5401cf8b34e5c",
            "88202c073303c41d7df6d03ec001a53a3d13f86e9265190b46d3c6587fab9e2773265f7165b131347a28346f2c74e3240cb46967626eb2aa22df61b9608ba66b"},
        {13,
            "02e5420afa905efcf10f46e61f5dddb19f47aca443fba979ded7310e1ac467dd11e5862f3cb474b78591d145cb6dabe302e87625d6e61d5e5b97962fabb2435e",
            "eba863b9003b8b805504bf8dd0f9fcd9950206d7a866a0eeb089130b84c36b803fe57eecafc25251ed3e68c57862298c988dbede2656ca6a82cbbbf0c9aefb3c"},
        {171,
            "23837067d9ae9a5f4dbe5c30ad86e28379a73276000d23fb096cb4a6bb406b3e5d7639e7c9650ee3814dec0d6a7d1b51ce10b41e2ddb9f8ea7fa2e6ac73320b1",
            "cdba9bccd4cb9bf7ecd48eb6ff3ae288cb56be91d25b8bb3b6681a93cc6ea254bc24ed0b1235fcbde16356ba2dc193ee8103d9dca681cf1aa4014911a71ac452"},
        {571,
            "922e3fc9b1741567df72b68531345e7f37c9ec64885101e587035ff5968db7163fc3d66851aa459911d432779acab42ca65c233d4d193e0fb39aedca1b8f7631",
            "9fa4cbdcb6b208f5cc86328a60657fb3818dab7fd496d5494ebda842902f251bce587a38928e5450d899899e769280df001a3bfc20b04a0ade4c6f6d82c8c174"},
        {620,
            "601668d81c7f8b0268d404f96528fa4d29226c3c84c4520f0d234d21e83be004571a8839826e102b4ef6c090480ef35a85f45700e5d2efbbb4c1721a797b2a39",
            "437fae4b69f3c25224c115cd74d600f008c3668c2815d27d2afb564d941412d0381ff7320ccc97c37238b6ebc54bf907dfccc6cf9ad8d9ca8662bda15c15f64b"},
        {514079,
            "259bc18e5cb2a4e5897c106fb9f3fe1480f8b48b86077673e3e9affa24f8ca5d3e307370ae56534aa8ebd88ade50aef7f9e941c173d18d189fc528bd5f6141b9",
            "6918d14a2fa8f7b0cf54297b949301770743b00a359ef428174dc8d2b8dc686fddfa79cd3bd9e46654d3848a37f9fe733e95b708132db1764eaa3b39db703040"},
        {852881,
            "95080490fe6781eda38979e23baba0915c7a4d441bd2a790fa930139ad99865ca890a06623b9ddfaaacc1fc17420b8d443ca53b89c4a3716aa5d6d088ca89a27",
            "5e639f40eb575f2677e1141a99e3bec90121067ffb7ed8340bdc889d8d128ea78414dc56f5148229d9fc1d5517352b6f96afd5d3a655095a05b85ce157bebd31"},
        {6133492,
            "cd7482a6294a547e770f02dd73c76a6606370cd1177b773aa1cd66868a76f3a817e0c39c76f54d8894ec7b5ebddf8606a0b20249f93d1e3defa325ba81aa4cd8",
            "c4a4dfcc85f6ef120871ab28e676806e6f080a0dd897fc648fb9dfd9df7fcae91e20c7f6d4ea161e15ec27542005dfc4293acab0365c98fec1f104b47b09f47f"},
        {6133493,
            "1a7d2bdfb9f60d83d86dc5404e1115ddee6029fbc48d854afbc18bedadeb6951d81391615d5d73d96e431f492dd66c5de56d4f71ca80915283170dcf9401d75b",
            "be530488cce0d1b7c1eca2361e43426960e413a93649aeec5dd74562da06cb1399e685c00705bea21f2ba4369b0fbcd0404052e4ea27aeab00f03343a38ab15c"},
        {211322105,
            "b7f04034d7a5ceb52074327a241b708ced97910b5a0f2b8e56464603ce678498659cf0db3866d39bbf8924d3efb5b23e4c2f5be6ff4b096a67f213e87137235f",
            "67ecd7b90665ad13c8de3092e798984413aac3bd37ab9f5a6583e341a7d2888679131f1afff253a6e0c524b2ec6dae8a0e607ea137e854e9ed2bb1ca572dd6aa"},
        {740620450,
            "7e4a3533ef6f0d9fa7e41b8304e08fe9e52556334cad0cc861337bd1155bbea211cf0b0198b4f08567cc47fcc964bbbdfb2f851437da1edba7c6f4bd3fd61a3a",
            "f20969bd0407bb76560e7c099224a1ea185214808950519fafdcd02ba2874e9b4ebf1797cafb3b80e903b13a87ddac5d54d67ed58acf49bb12e03b81eb6c99af"},
        {4294967295,
            "21471504c1f31007c14acd107a8ade1aad6c2a6c2ad879b3aca3b12517105483502d0e3e902acf3b128d294c0a69f2cc199bf8813be1f8bb4b5625822b70ec09",
            "8e4fdb5dc602598f10a42b5061132eec05299380db872a3caf04aa21e3d4970350394dfbd58c5ab54571b1be0cc9001d788c6b14cbf003d7decc2aaef1232b8c"},
    };
    // clang-format on

    // Mock the epoch context.
    epoch_context context = create_epoch_context_mock(0);

    for (const auto& t : test_cases)
    {
        const hash1024 item = calculate_dataset_item(context, t.index);
        EXPECT_EQ(to_hex(item.hashes[0]), t.hash1_hex) << "index: " << t.index;
        EXPECT_EQ(to_hex(item.hashes[1]), t.hash2_hex) << "index: " << t.index;
    }
}


TEST(ethash, dataset_items_epoch13)
{
    struct full_dataset_item_test_case
    {
        size_t index;
        const char* hash1_hex;
        const char* hash2_hex;
    };

    // clang-format off
    full_dataset_item_test_case test_cases[] = {
        {0,
            "6526a7e7ab092dfd21ebe6ffa1225370886a9a9fc828f929775de6a2737a3b9d1bb728bd23a61478e9fbf837d317829292d469cbb1a90a29058c0f604aeee5a6",
            "a2fe2a6253d8642b5dcc6a211b7f4d6ee9b60ac235ee2c9d64a064ea20e47fe4cdad99aa9c7dd5e2d71e58b7957ff91062d1f1c014cd4453db2ddb094056529b"},
        {1,
            "7a03a5f8e028db852542ce2f10d25b7dcdf9a25c3898aabe78b881ef58e56ffaedb24c870184a6f0513a65f09066e9f9224baf6f684e0c54e7ef400004ec9fb8",
            "639f251bbee0b1114e687ea651717eb69c0b76bc673dd4c9d697f41c307d66bdbe92b26df3dda209f25d6784c7a1db5902c18323285255684359728f5ca6da31"},
        {13,
            "63886d5b8967584addad94d45d194209bdf05e34b11894575ad292bf6da94117224b6e0d7b57791438ff701d3a0d2a80f261cc2ebde97e3bfc8574bb3519593c",
            "afec130195473d4389d0ea13ec33ba8048a4075043fd2f2bc502f534f4f600ddb3417e53a9f23918c5e134049ed2b8561f1165d8543852f7e9792fa6332c268c"},
        {171,
            "a3dabfb475532cbc59d8b6732e73f108bba7b325b90e32690f00ea08f8977a94774106b08314763d403b153bc27fb58d5b2b84955e3069e2e6e63718bc2d3a26",
            "056c1c4861d530aa6aa962cfd8256828661c93a3985d4dfa88011b496e94b031e86650183c706488984d0d0302ed79d5c1ac977dc6881006cb2b0b6b76542067"},
        {571,
            "14276428b23db295620e56dc2212c4f75f4dc20545ad0d2fe5f08e7c99b5fb372bb342f098525e30ff49f2b66891d9973101c8b4579269311afbc6c0a8822bbd",
            "1ed369ce428c0c768c9fcc45089e830e603baa3b6fb81f60ef4b652ac19ab90ce3b3b007fb5fc289bc822e82e3b6b9aaef51258cd6bfecf23c30ed576a246d6a"},
        {620,
            "d3df66afb0b84a35b6ddb32d1b04a30c814cad357c0e46bffa2f6cbf0d95c76eef10b3b59fdce344924981cfdacdf93accecfa6ffe8b5041640e83feccb26a9c",
            "fb97636b8e8b9bb593ab2d0235bc803608901cb3484e277761d376baf03d6986caf24f00a37a5a970a5b0de551cc27d45b9bf6ce3986b18cd78d98c028e956b1"},
        {514079,
            "279fe53aec52daf6341580b4c43841b39157b66144733ddaa3b11886a9bc3a61892af817e0f1ff9ac374351aa7cc16f7ac55bbc0cd53aaa72747db9fe07d9938",
            "793b56c9baafe3638d9b3bfdf09f136d9b01bf206919df571e3e6de7320822f6d1567c69240b28181d797d0ab53b342f17dcb9746bdc0afb75fd115e7aa244ec"},
        {852881,
            "9f877290e5ba25760b6719e12c06e7e8e36b5d2510354ec3e269e04c36146bd8c7f5a1aa4df47c7a0f1f139bdfb5a1a7108b7eadd41d7acb40532c4cff1619d1",
            "a0b6e119086c1f26aa578c678374e59b49388941fe9a119d00a7c82214702d5626b810df756fb399a5cf54cb09ce6aa25ed99ebe3bcc92178c9180cf956189a5"},
        {6133492,
            "539f72291dd31e5234081057904daff0329bf0099726c20fa1f1b31377ac84acc60d64c29206ccc0191a5f797293c151f0d506fa2ef5daf70654654c349fbf70",
            "764e27dd47579073a74f92ef843a11219629583ab6b9b4907dcededbacbea1ed8b3f7a36336dc9a39839faf4d5095e50d9632e9e23819d7f85daa0f87a6a2efa"},
        {6133493,
            "379a030e94c3718a7220dcc9865e600eea2bb049aa251e26d812c952fa9bedecfc2e9a04d421810d5ef2500e9d5a4a44d8d6e731414a7bd94268333f23368c59",
            "609f6d9d852794d11cf748bdb1784662ee0080fed4c18683a74ef4fdb3a5c243c9b3981b87fe912ac8860ff1a021e115dae2c96f14de6387c802c9910290ead3"},
        {211322105,
            "b3bb7286d9840720319aa6dd4bc7f8b55cc38b1ff0151c4a46ffb4b2a99c6eeb966bde5efc7f3f0c3a753c338238e37e119b75e6c553f2aab575238e1ba07880",
            "0b89a30d5ed6bf1959c6e4d265ddf941bed6ce579493726c7d98de68267bb0eccfb0dbdf7a2d0d45d900d7588995212a8618ea679ab08425922b5042a0fdb95c"},
        {740620450,
            "df0c2e2f4df033a64b1bcd207c30c7ce48c7d8ca8edd1284c87a91d54372ed0cb513d1876b1dbef6fc06c496941039cba6c50676596d6379152689d9841c97e4",
            "357bacef5baf4687c87e7ff07d5ab104ce39badcf9633c22ee31c3c3de0887b296f9385ea27573cb94bc3423cc39ab2a733be97a98e860290c31e94f03f39814"},
        {4294967295,
            "11fab5bafdf0e29f199cad053a542f777fcd8b4fb8a0203bf720b9a01718e8c76d0e374e979ebf0e1faf8ce992638a5e92ea8be8000c47e8307acad261df1abb",
            "164ff9a893a162319f9ccb4294e33fb6ae50ea05d02a753fd4797662676c1fad6d70b11db6d4aa0298d6aa695c9be8dea3dad70f953368cb11b283eb145d17e3"},
    };
    // clang-format on


    // Create example epoch context.
    epoch_context* context = create_epoch_context(13);

    for (const auto& t : test_cases)
    {
        const hash1024 item = calculate_dataset_item(*context, t.index);
        EXPECT_EQ(to_hex(item.hashes[0]), t.hash1_hex) << "index: " << t.index;
        EXPECT_EQ(to_hex(item.hashes[1]), t.hash2_hex) << "index: " << t.index;
    }

    destroy_epoch_context(context);
}

TEST(ethash, verify_hash_light)
{
    for (const auto& t : hash_test_cases)
    {
        const int epoch_number = t.block_number / epoch_length;
        const uint64_t nonce = std::stoull(t.nonce_hex, nullptr, 16);
        const hash256 header_hash = to_hash256(t.header_hash_hex);
        const hash256 mix_hash = to_hash256(t.mix_hash_hex);
        const std::string target_hex{t.final_hash_hex, 16};
        const uint64_t target = std::stoull(target_hex, nullptr, 16) + 1;

        epoch_context* context = create_epoch_context(epoch_number);

        result r = hash_light(*context, header_hash, nonce);
        EXPECT_EQ(to_hex(r.final_hash), t.final_hash_hex);
        EXPECT_EQ(to_hex(r.mix_hash), t.mix_hash_hex);

        bool v = verify(*context, header_hash, mix_hash, nonce, target);
        EXPECT_TRUE(v);

        destroy_epoch_context(context);
    }
}

TEST(ethash, verify_hash)
{
    for (const auto& t : hash_test_cases)
    {
        const int epoch_number = t.block_number / epoch_length;
        const uint64_t nonce = std::stoull(t.nonce_hex, nullptr, 16);
        const hash256 header_hash = to_hash256(t.header_hash_hex);

        epoch_context* context = create_epoch_context(epoch_number);
        init_full_dataset(*context);

        result r = hash(*context, header_hash, nonce);
        EXPECT_EQ(to_hex(r.final_hash), t.final_hash_hex);
        EXPECT_EQ(to_hex(r.mix_hash), t.mix_hash_hex);

        destroy_epoch_context(context);
    }
}

TEST(ethash_multithreaded, small_dataset)
{
    // This test creates an extremely small dataset for full search to discover
    // sync issues between threads.

    constexpr size_t num_treads = 8;
    constexpr size_t num_dataset_items = 501;
    constexpr uint64_t target = uint64_t(1) << 50;

    epoch_context context = create_epoch_context_mock(0);
    context.full_dataset_size = num_dataset_items * sizeof(hash1024);
    init_full_dataset(context);

    std::array<std::future<uint64_t>, num_treads> futures;
    for (auto& f : futures)
        f = std::async(std::launch::async, [&] { return search(context, {}, target, 1, 11000); });

    for (auto& f : futures)
        EXPECT_EQ(f.get(), 10498);
}

TEST(ethash, small_dataset_light)
{
    constexpr size_t num_dataset_items = 501;
    constexpr uint64_t target = uint64_t(1) << 55;

    epoch_context context = create_epoch_context_mock(0);
    context.full_dataset_size = num_dataset_items * sizeof(hash1024);

    uint64_t solution = search_light(context, {}, target, 475, 10);
    EXPECT_EQ(solution, 482);

    solution = search_light(context, {}, target, 483, 10);
    EXPECT_EQ(solution, 0);
}

TEST(ethash, init_full_dataset_oom)
{
    auto mock_context = create_epoch_context_mock(0);

    constexpr uint64_t max = std::numeric_limits<size_t>::max();
    uint64_t big_size = uint64_t(1) << 40;
    size_t size = static_cast<size_t>(std::min(big_size, max));
    mock_context.full_dataset_size = size;
    EXPECT_FALSE(init_full_dataset(mock_context));
}
