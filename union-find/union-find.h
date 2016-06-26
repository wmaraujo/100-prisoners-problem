typedef struct {
    int p[100];    // parent element
    int size[100]; // num of elements in subtree i
    int n;           // num of elements in set
} set_union;

void set_union_init(set_union* s, int n);
int find(set_union* s, int x);
void union_set(set_union* s, int s1, int s2);
unsigned char same_component(set_union* s, int s1, int s2);
