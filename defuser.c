#include "defuser.h"

#include <inttypes.h>
#include <stdio.h>

#define MAX_FIELDS 8

/*
 * 把 values 中的 count 个整数写成一行“空格分隔的无符号十进制整数”，存入 out。
 * 成功返回 0；count 为 0（还没有实现）、snprintf 出错或结果被截断时返回非零。
 */
static int write_answer(char *out, size_t cap, const uint32_t *values, size_t count) {
    size_t used = 0;
    if (!out || !values || !cap || !count) return 1;
    for (size_t i = 0; i < count; ++i) {
        int s = snprintf(out + used, cap - used, "%s%" PRIu32, i ? " " : "", values[i]);
        if (s < 0 || (size_t)s >= cap - used) return 1;
        used += (size_t)s;
    }
    return 0;
}

int solve_phase_1(uint64_t challenge, char *out, size_t cap) {
    uint32_t low = (uint32_t)challenge, high = (uint32_t)(challenge >> 32);
    uint32_t a = (low + UINT32_C(0x565f)) ^ high; 
    uint32_t b = (low * UINT32_C(3)) ^ ((uint32_t)(challenge >> 16) + UINT32_C(0x65f8));
    uint32_t values[] = {b * UINT32_C(8) + (a ^ UINT32_C(0x5ea9))};
    return write_answer(out, cap, values, 1);
}

int solve_phase_2(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[6];
    values[0] = (((uint32_t)(challenge >> 44) * UINT32_C(2)) & UINT32_C(255)) + UINT32_C(11);
    for (unsigned i = 1; i < 6; ++i) {
        uint32_t t = (uint32_t)(challenge >> (13 + 2 * i)) & UINT32_C(63);
        values[i] = values[i - 1] * UINT32_C(5) + (t ^ (i + UINT32_C(5)));
    }
    return write_answer(out, cap, values, 6);
}

int solve_phase_3(uint64_t challenge, char *out, size_t cap) {
    static const unsigned cases[8] = {1, 6, 2, 3, 5, 0, 7, 4};
    uint32_t index = (uint32_t)challenge & UINT32_C(7);
    uint32_t x = (uint32_t)(challenge >> 3) ^ (uint32_t)(challenge >> 32), r;
    switch (cases[index]) {
        case 0: r = x * UINT32_C(5) + UINT32_C(0x7d78); break;
        case 1: r = (x * UINT32_C(2)) ^ UINT32_C(0x39f1); break;
        case 2: r = (x ^ UINT32_C(0xc35f)) + (x >> 2); break;
        case 3: r = x * UINT32_C(5) + UINT32_C(0x5ad5); break;
        case 4: r = x * UINT32_C(3) + UINT32_C(0x2ec4); break;
        case 5: r = (x * UINT32_C(2)) ^ UINT32_C(0x0be1); break;
        case 6: r = (x ^ UINT32_C(0xdab3)) + (x >> 2); break;
        default: r = x * UINT32_C(5) + UINT32_C(0x3a7f5); break;
    }
    uint32_t values[] = {index, r};
    return write_answer(out, cap, values, 2);
}

static uint32_t walk(uint32_t target, uint32_t lo, uint32_t hi, uint32_t key) {
    uint32_t mid = lo + ((hi - lo) >> 1);
    if (target == mid) return (mid + UINT32_C(0x63)) ^ key;
    if (target < mid)
        return (walk(target, lo, mid - 1, key) * UINT32_C(3))
             ^ (mid + key) ^ UINT32_C(0x1d);
    return (walk(target, mid + 1, hi, key) * UINT32_C(3))
         ^ (mid + key) ^ UINT32_C(0x0b);
}

int solve_phase_4(uint64_t challenge, char *out, size_t cap) {
    uint32_t key = (uint32_t)(challenge >> 7) ^ (uint32_t)(challenge >> 32);
    uint32_t values[] = {walk((uint32_t)challenge & UINT32_C(127), 0, 127, key)};
    return write_answer(out, cap, values, 1);
}

int solve_phase_5(uint64_t challenge, char *out, size_t cap) {
    static const struct { uint32_t value, id; unsigned next; } nodes[12] = {
        {0xc07c, 0x814, 11}, {0x92b6, 0x675, 8}, {0x0505, 0xb22, 7},
        {0x39c5, 0x007, 1}, {0x1151, 0x982, 2}, {0x4165, 0x324, 9},
        {0x1129, 0x263, 4}, {0xa989, 0x782, 5}, {0x0aca, 0x429, 12},
        {0x068a, 0x569, 0}, {0x3101, 0x161, 3}, {0xd99f, 0xa16, 10}
    };
    static const unsigned heads[8] = {6, 4, 2, 7, 5, 9, 0, 11};
    unsigned node = heads[(unsigned)challenge & 7], hops = (unsigned)(challenge >> 3) & 3;
    uint32_t acc = (uint32_t)(challenge >> 6) ^ (uint32_t)(challenge >> 32) ^ UINT32_C(0x857f);
    uint32_t id = 0;
    for (unsigned i = 0; i <= hops; ++i) {
        if (node >= 12) return 1;
        id = nodes[node].id;
        acc = (acc * UINT32_C(3) + nodes[node].value) ^ id;
        node = nodes[node].next;
    }
    uint32_t values[] = {acc, id};
    return write_answer(out, cap, values, 2);
}

static uint32_t operation(unsigned index, uint32_t x) {
    static const unsigned ops[8] = {0, 0, 4, 1, 1, 3, 2, 5};
    switch (ops[index]) {
        case 0: return (x ^ UINT32_C(0x23c1)) + x * UINT32_C(4);
        case 1: return (x >> 2) | (x << 30);
        case 2: return ((x << 7) + UINT32_C(0xe07c)) ^ (x * UINT32_C(3));
        case 3: return ((x << 1) | (x >> 31)) ^ UINT32_C(0x91d8);
        case 4: return ((x >> 2) ^ x) + UINT32_C(0xd265);
        default: return (x + UINT32_C(0x809f)) ^ (x >> 2);
    }
}

int solve_phase_6(uint64_t challenge, char *out, size_t cap) {
    uint32_t x = (uint32_t)(challenge >> 6) ^ (uint32_t)(challenge >> 32);
    uint32_t a = operation((unsigned)challenge & 7, x);
    uint32_t values[] = {operation((unsigned)(challenge >> 3) & 7, a ^ UINT32_C(0x6aed))
                    + UINT32_C(0xe0de)};
    return write_answer(out, cap, values, 1);
}

int solve_secret(uint64_t challenge, uint64_t context, char *out, size_t cap) {
    uint32_t k = (uint32_t)context & UINT32_C(3);
    uint32_t a = (uint32_t)challenge ^ (uint32_t)(challenge >> 32);
    uint32_t b = (uint32_t)context ^ (uint32_t)(context >> 32), x = a ^ b;
    uint32_t mix = ((x >> 1) ^ UINT32_C(0x95a3)) + (x ^ UINT32_C(0x3199));
    uint32_t second = mix ^ (k * UINT32_C(0x3199) ^ (uint32_t)(context >> 16));
    uint32_t third = second ^ (b + k + UINT32_C(0x95a3) + (a >> 1));
    
    if (!out || !cap) return 1;
    int s = snprintf(out, cap, "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAOPEN%" PRIu32
                     " %" PRIu32 " %" PRIu32, k, second, third);
    return s < 0 || (size_t)s >= cap;
}
