#include "union-find.h"

void set_union_init(set_union* s, int n) {
    for (int i = 0; i < n; i++) {
        s->p[i] = i;
        s->size[i] = 1;
    }
    s->n = n;
}

int find(set_union* s, int x) {
    if (s->p[x] == x) return x;
    else {
        s->p[x] = s->p[s->p[x]]; // semi-path compression
        return find(s, s->p[x]);
    }
}

void union_set(set_union* s, int s1, int s2) {
    int r1 = find(s, s1);
    int r2 = find(s, s2);

    if (r1 == r2) return;

    if (s->size[r1] >= s->size[r2]) {
        s->size[r1] = s->size[r1] + s->size[r2];
        s->p[r2] = r1;
    }
    else {
        s->size[r2] = s->size[r1] + s->size[r2];
        s->p[r1] = r2;
    }
}

unsigned char same_component(set_union* s, int s1, int s2) {
    return find(s, s1) == find(s, s2);
}
