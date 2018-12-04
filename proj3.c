/**
 * Kostra programu pro 3. projekt IZP 2017/18
 *
 * Jednoducha shlukova analyza
 * Unweighted pair-group average
 * https://is.muni.cz/th/172767/fi_b/5739129/web/web/usrov.html
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h> // sqrtf
#include <limits.h> // INT_MAX
#include <string.h>


#define SIZE 1024
/*****************************************************************
 * Ladici makra. Vypnout jejich efekt lze definici makra
 * NDEBUG, napr.:
 *   a) pri prekladu argumentem prekladaci -DNDEBUG
 *   b) v souboru (na radek pred #include <assert.h>
 *      #define NDEBUG
 */
#ifdef NDEBUG
#define debug(s)
#define dfmt(s, ...)
#define dint(i)
#define dfloat(f)
#else

// vypise ladici retezec
#define debug(s) printf("- %s\n", s)

// vypise formatovany ladici vystup - pouziti podobne jako printf
#define dfmt(s, ...) printf(" - "__FILE__":%u: "s"\n",__LINE__,__VA_ARGS__)

// vypise ladici informaci o promenne - pouziti dint(identifikator_promenne)
#define dint(i) printf(" - " __FILE__ ":%u: " #i " = %d\n", __LINE__, i)

// vypise ladici informaci o promenne typu float - pouziti
// dfloat(identifikator_promenne)
#define dfloat(f) printf(" - " __FILE__ ":%u: " #f " = %g\n", __LINE__, f)

#endif

/*****************************************************************
 * Deklarace potrebnych datovych typu:
 *
 * TYTO DEKLARACE NEMENTE
 *
 *   struct obj_t - struktura objektu: identifikator a souradnice
 *   struct cluster_t - shluk objektu:
 *      pocet objektu ve shluku,
 *      kapacita shluku (pocet objektu, pro ktere je rezervovano
 *          misto v poli),
 *      ukazatel na pole shluku.
 */

struct obj_t {
    int id;
    float x;
    float y;
};

struct cluster_t { //shluk objektu
    int size; //pocet objetku ve shluku - v cluster_t
    int capacity; //kapacita cluster_t (pocet obj_t, pro ktere je rezervovano misto v poli)
    struct obj_t *obj; //ukazatel na pole shluku
};

/*****************************************************************
 * Deklarace potrebnych funkci.
 *
 * PROTOTYPY FUNKCI NEMENTE
 *
 * IMPLEMENTUJTE POUZE FUNKCE NA MISTECH OZNACENYCH 'TODO'
 *
 */

/*
 Inicializace shluku 'c'. Alokuje pamet pro cap objektu (kapacitu).
 Ukazatel NULL u pole objektu znamena kapacitu 0.
*/
void init_cluster(struct cluster_t *c, int cap)
{
    assert(c != NULL);
    assert(cap >= 0);

    c->size = 0;
    if(cap>0){
        c->obj = malloc(sizeof(struct obj_t)*cap);
        c->capacity = cap;
    }else{
        c->capacity = 0;
        c->obj = NULL;
    }
}


/*
 Odstraneni vsech objektu shluku a inicializace na prazdny shluk.
 */
void clear_cluster(struct cluster_t *c)
{
    if(c !=NULL){
        free(c->obj);
        c->obj = NULL;
        c->size = 0;
        c->capacity = 0; 
    }
}

/// Chunk of cluster objects. Value recommended for reallocation.
const int CLUSTER_CHUNK = 10;

/*
 Zmena kapacity shluku 'c' na kapacitu 'new_cap'.
 */
struct cluster_t *resize_cluster(struct cluster_t *c, int new_cap)
{
    // TUTO FUNKCI NEMENTE
    assert(c);
    assert(c->capacity >= 0);
    assert(new_cap >= 0);

    if (c->capacity >= new_cap)
        return c;

    size_t size = sizeof(struct obj_t) * new_cap;

    void *arr = realloc(c->obj, size);
    if (arr == NULL)
        return NULL;

    c->obj = (struct obj_t*)arr;
    c->capacity = new_cap;
    return c;
}

/*
 Prida objekt 'obj' na konec shluku 'c'. Rozsiri shluk, pokud se do nej objekt
 nevejde.
 */
void append_cluster(struct cluster_t *c, struct obj_t obj)
{
    if(c != NULL){
        if(c->size == c->capacity){
            c = resize_cluster(c, c->capacity+1);
        }
        if (c->size < c->capacity)
        {
            c->obj[c->size] = obj;
            c->size ++;
        }
    }
}

/*
 Seradi objekty ve shluku 'c' vzestupne podle jejich identifikacniho cisla.
 */
void sort_cluster(struct cluster_t *c);

/*
 Do shluku 'c1' prida objekty 'c2'. Shluk 'c1' bude v pripade nutnosti rozsiren.
 Objekty ve shluku 'c1' budou serazeny vzestupne podle identifikacniho cisla.
 Shluk 'c2' bude nezmenen.
 */
void merge_clusters(struct cluster_t *c1, struct cluster_t *c2)
{
    assert(c1 != NULL);
    assert(c2 != NULL);

    if(c2->capacity > c1->capacity){
        c1 = resize_cluster(c1,c2->capacity+c1->size);
    }
    
    for (int i = 0; i < c2->size; i++) {
		append_cluster(c1, c2->obj[i]);
	}

    sort_cluster(c1);
}

/**********************************************************************/
/* Prace s polem shluku */

/*
 Odstrani shluk z pole shluku 'carr'. Pole shluku obsahuje 'narr' polozek
 (shluku). Shluk pro odstraneni se nachazi na indexu 'idx'. Funkce vraci novy
 pocet shluku v poli.
*/
int remove_cluster(struct cluster_t *carr, int narr, int idx)
{
    assert(idx < narr);
    assert(narr > 0);

    clear_cluster(&carr[idx]);

    return narr - 1;
}

/*
 Pocita Euklidovskou vzdalenost mezi dvema objekty.
 */
float obj_distance(struct obj_t *o1, struct obj_t *o2)
{
    assert(o1 != NULL);
    assert(o2 != NULL);
    
    if(o1 != NULL && o2 != NULL){
        float distance = 0.0;
        distance = sqrtf(powf((o2->x) - (o1->x),2.0) + powf((o2->y) - (o1->y),2.0));
        
        return distance;
    }
    
    return 0;

}

/*
 Pocita vzdalenost dvou shluku.
*/
float cluster_distance(struct cluster_t *c1, struct cluster_t *c2)
{
    assert(c1 != NULL);
    assert(c1->size > 0);
    assert(c2 != NULL);
    assert(c2->size > 0);

    float distance = 0.0;

    distance = obj_distance(c1->obj,c2->obj);

    return distance;
}

/*
 Funkce najde dva nejblizsi shluky. V poli shluku 'carr' o velikosti 'narr'
 hleda dva nejblizsi shluky. Nalezene shluky identifikuje jejich indexy v poli
 'carr'. Funkce nalezene shluky (indexy do pole 'carr') uklada do pameti na
 adresu 'c1' resp. 'c2'.
*/
void find_neighbours(struct cluster_t *carr, int narr, int *c1, int *c2)
{
    assert(narr > 0);
    if(narr > 0){
        float cluster_dist = 0.0;
        cluster_dist = cluster_distance(carr + 1, carr+2);
        *c1 = 1;
        *c2 = 2;
    }
}

// pomocna funkce pro razeni shluku
static int obj_sort_compar(const void *a, const void *b)
{
    // TUTO FUNKCI NEMENTE
    const struct obj_t *o1 = (const struct obj_t *)a;
    const struct obj_t *o2 = (const struct obj_t *)b;
    if (o1->id < o2->id) return -1;
    if (o1->id > o2->id) return 1;
    return 0;
}

/*
 Razeni objektu ve shluku vzestupne podle jejich identifikatoru.
*/
void sort_cluster(struct cluster_t *c)
{
    // TUTO FUNKCI NEMENTE
    qsort(c->obj, c->size, sizeof(struct obj_t), &obj_sort_compar);
}
void print_clusters(struct cluster_t *carr, int narr);
/*
 Tisk shluku 'c' na stdout.
*/
void print_cluster(struct cluster_t *c)
{
    // TUTO FUNKCI NEMENTE
    for (int i = 0; i < c->size; i++)
    {
        if (i) putchar(' ');
        printf("%d[%g,%g]", c->obj[i].id, c->obj[i].x, c->obj[i].y);
    }
    putchar('\n');
}

/*
 Ze souboru 'filename' nacte objekty. Pro kazdy objekt vytvori shluk a ulozi
 jej do pole shluku. Alokuje prostor pro pole vsech shluku a ukazatel na prvni
 polozku pole (ukalazatel na prvni shluk v alokovanem poli) ulozi do pameti,
 kam se odkazuje parametr 'arr'. Funkce vraci pocet nactenych objektu (shluku).
 V pripade nejake chyby uklada do pameti, kam se odkazuje 'arr', hodnotu NULL.
*/
int load_clusters(char *filename, struct cluster_t **arr)
{
    assert(arr != NULL);

    *arr = NULL;

    FILE *soubor;
    soubor = fopen(filename, "r");
    char line[SIZE];
    int count = 0;


    struct obj_t object;
    struct cluster_t *clusters = NULL;

    if(soubor != NULL){
        fscanf(soubor,"%[^\n]\n",line);
        if(strstr(line, "count=") != NULL){
            sscanf(line, "%*[^0-9]%d", &count);

            clusters = malloc(sizeof(struct cluster_t)*count);
            if(clusters == NULL){
                fprintf(stderr,"problem pri alokaci pameti\n");
            }
            count = 8;
            for(int i=0;i<count;i++){
                fscanf(soubor,"%d %g %g \n",&object.id, &object.x, &object.y);
                init_cluster((clusters+i), 1);
                append_cluster((clusters+i), object);
            }
            arr = &clusters;
            //print_clusters(*arr,count);
        }else{
            fprintf(stderr, "Chyba v prvnim radku souboru\n");
            exit(1);
        }
    }else{
        fprintf(stderr, "soubor se nepodarilo nacist\n");
        exit(1);
    }

    count = 8;
    //printf("%g\n",cluster_distance((clusters+0), (clusters+1)));
    float clus_distance = INT_MAX;
    float clus_distance_new = 0.0;
    int position = 0;

    for(int j = 0; j<count;j++){
        for(int i = 0; i <count-1;i++){
            clus_distance_new = cluster_distance((clusters+j), (clusters+i+1));
            if(clus_distance > clus_distance_new){
                clus_distance = clus_distance_new;
                position = i;
            }
        }
        merge_clusters((clusters+j), (clusters+position));
        resize_cluster((clusters+position),0);
        clear_cluster((clusters+position));
        remove_cluster(clusters,count,position);
    }
    print_clusters(clusters,count);
    return 0;
}

/*
 Tisk pole shluku. Parametr 'carr' je ukazatel na prvni polozku (shluk).
 Tiskne se prvnich 'narr' shluku.
*/
void print_clusters(struct cluster_t *carr, int narr)
{
    printf("Clusters:\n");
    for (int i = 0; i < narr; i++)
    {
        printf("cluster %d: ", i);
        print_cluster(&carr[i]);
    }
}



int main(int argc, char *argv[])
{
    struct cluster_t *clusters;

    if(argc <= 1 || argc > 3){
        fprintf(stderr, "neplatny argument\n");
        return 1;
    }
    load_clusters(argv[1], &clusters);

    free(clusters);


    return 0;
}