///////////////////////////////////////////////////////////
//
// Υλοποίηση του ADT Vector μέσω Dynamic Array.
//
///////////////////////////////////////////////////////////

#include <stdlib.h>
#include <assert.h>

#include "ADTVector.h"

// Το αρχικό μέγεθος που δεσμεύουμε
#define VECTOR_MIN_CAPACITY 10

// Ένα VectorNode είναι pointer σε αυτό το struct. (το struct περιέχει μόνο ένα στοιχείο, οπότε θα μπροούσαμε και να το αποφύγουμε, αλλά κάνει τον κώδικα απλούστερο)
struct vector_node
{
    Pointer value; // Η τιμή του κόμβου.
};

// Ενα Vector είναι pointer σε αυτό το struct
struct vector
{
    VectorNode array; // Τα δεδομένα, πίνακας από struct vector_node
    VectorNode new_array;
    int size;     // Πόσα στοιχεία έχουμε προσθέσει
    int capacity; // Πόσο χώρο έχουμε δεσμεύσει (το μέγεθος του array). Πάντα capacity >= size, αλλά μπορεί να έχουμε
    bool resize;
    int index_of_old_array;
    DestroyFunc destroy_value; // Συνάρτηση που καταστρέφει ένα στοιχείο του vector.
};

Vector vector_create(int size, DestroyFunc destroy_value)
{
    // Δημιουργία του struct
    Vector vec = malloc(sizeof(*vec));

    vec->size = size;
    vec->destroy_value = destroy_value;

    // Δέσμευση μνήμης για τον πίνακα. Αρχικά το vector περιέχει size
    // μη-αρχικοποιημένα στοιχεία, αλλά εμείς δεσμεύουμε xώρο για τουλάχιστον
    // VECTOR_MIN_CAPACITY για να αποφύγουμε τα πολλαπλά resizes.
    //
    vec->capacity = size < VECTOR_MIN_CAPACITY ? VECTOR_MIN_CAPACITY : size;
    vec->array = calloc(vec->capacity, sizeof(*vec->array)); // αρχικοποίηση σε 0 (NULL)
    vec->resize = false;
    vec->index_of_old_array = 0;

    return vec;
}

int vector_size(Vector vec)
{
    return vec->size;
}

Pointer vector_get_at(Vector vec, int pos)
{
    assert(pos >= 0 && pos < vec->size); // LCOV_EXCL_LINE (αγνοούμε το branch από τα coverage reports, είναι δύσκολο να τεστάρουμε το false γιατί θα κρασάρει το test)

    if (pos >= vec->capacity / 2 && vec->resize == true)
    {
        return vec->new_array[pos].value;
    }
    else
    {
        return vec->array[pos].value;
    }
}

void vector_set_at(Vector vec, int pos, Pointer value)
{
    assert(pos >= 0 && pos < vec->size); // LCOV_EXCL_LINE

    // Αν υπάρχει συνάρτηση destroy_value, την καλούμε για το στοιχείο που αντικαθίσταται
    if (pos >= vec->capacity / 2 && vec->resize == true)
    {
        if (value != vec->new_array[pos].value && vec->destroy_value != NULL)
            vec->destroy_value(vec->new_array[pos].value);

        vec->new_array[pos].value = value;
    }
    else
    {
        if (value != vec->array[pos].value && vec->destroy_value != NULL)
            vec->destroy_value(vec->array[pos].value);

        vec->array[pos].value = value;
    }
}

void vector_insert_last(Vector vec, Pointer value)
{
    // Μεγαλώνουμε τον πίνακα (αν χρειαστεί), ώστε να χωράει τουλάχιστον size στοιχεία
    // Διπλασιάζουμε κάθε φορά το capacity (σημαντικό για την πολυπλοκότητα!)
    if (vec->capacity == vec->size)
    {
        // Προσοχή: δεν πρέπει να κάνουμε free τον παλιό pointer, το κάνει η realloc
        vec->capacity *= 2;
        // vec->array = realloc(vec->array, vec->capacity * sizeof(*vec->array));
        //φτιάχνω νέο array διπλάσιου capacity
        vec->new_array = calloc(vec->capacity, sizeof(*vec->array));
        vec->resize = true;
        //αντιγράφω στοιχείο στο vec->size και στο i το οποίο ξεκινάει από το 0
        //όταν i = size / 2(βρες το) διαγράφω το vec->array(free)/destroy_value
        //vec->array = new_array
        //free/destroy_value(new_array)
        //κάνε παλι το 80
    }

    if (vec->resize == true)
    {
        vec->new_array[vec->size].value = value;
        vec->size++;
        vec->new_array[vec->index_of_old_array].value = vec->array[vec->index_of_old_array].value;
        vec->index_of_old_array++;

        if (vec->capacity == vec->size)
        {
            vec->resize = false;
            free(vec->array);
            vec->array = vec->new_array;
            // free(vec->new_array);
            vec->index_of_old_array = 0;
        }
    }
    else
    {
        // Μεγαλώνουμε τον πίνακα και προσθέτουμε το στοιχείο
        vec->array[vec->size].value = value;
        vec->size++;
    }
}

void vector_remove_last(Vector vec)
{
    assert(vec->size != 0); // LCOV_EXCL_LINE

    // Αν υπάρχει συνάρτηση destroy_value, την καλούμε για το στοιχείο που αφαιρείται

    if (vec->resize == true)
    {
        if (vec->destroy_value != NULL)
            vec->destroy_value(vec->new_array[vec->size - 1].value);
    }
    else
    {
        if (vec->destroy_value != NULL)
            vec->destroy_value(vec->array[vec->size - 1].value);
    }

    // Αφαιρούμε στοιχείο οπότε ο πίνακας μικραίνει
    vec->size--;

    // Μικραίνουμε τον πίνακα αν χρειαστεί, ώστε να μην υπάρχει υπερβολική σπατάλη χώρου.
    // Για την πολυπλοκότητα είναι σημαντικό να μειώνουμε το μέγεθος στο μισό, και μόνο
    // αν το capacity είναι τετραπλάσιο του size (δηλαδή το 75% του πίνακα είναι άδειος).
    //
    if (vec->capacity > vec->size * 4 && vec->capacity > 2 * VECTOR_MIN_CAPACITY)
    {
        vec->capacity /= 2;
        if (vec->new_array != NULL)
            free(vec->new_array);
        vec->resize = false;
        // vec->array = realloc(vec->array, vec->capacity * sizeof(*vec->array));
    }
}

Pointer vector_find(Vector vec, Pointer value, CompareFunc compare)
{
    // Διάσχιση του vector
    for (int i = 0; i < vec->size; i++)
        if (compare(vec->array[i].value, value) == 0)
            return vec->array[i].value; // βρέθηκε

    return NULL; // δεν υπάρχει
}

DestroyFunc vector_set_destroy_value(Vector vec, DestroyFunc destroy_value)
{
    DestroyFunc old = vec->destroy_value;
    vec->destroy_value = destroy_value;
    return old;
}

void vector_destroy(Vector vec)
{
    // Αν υπάρχει συνάρτηση destroy_value, την καλούμε για όλα τα στοιχεία
    if (vec->destroy_value != NULL)
        for (int i = 0; i < vec->size; i++)
            vec->destroy_value(vec->array[i].value);

    // Πρέπει να κάνουμε free τόσο τον πίνακα όσο και το struct!
    free(vec->array);
    free(vec->new_array);
    free(vec); // τελευταίο το vec!
}

// Συναρτήσεις για διάσχιση μέσω node /////////////////////////////////////////////////////

VectorNode vector_first(Vector vec)
{
    if (vec->size == 0)
        return VECTOR_BOF;
    else
        return &vec->array[0];
}

VectorNode vector_last(Vector vec)
{
    if (vec->size == 0)
        return VECTOR_EOF;
    else
        return &vec->new_array[vec->size - 1];
}

VectorNode vector_next(Vector vec, VectorNode node)
{
    if (node == &vec->new_array[vec->size - 1])
        return VECTOR_EOF;
    else
    {
        VectorNode next = node + 1;
        if (next->value == NULL)
        {
            next = &vec->new_array[vec->capacity / 2];
            return next;
        }
        else
        {
            return next;
        }
    }
}

VectorNode vector_previous(Vector vec, VectorNode node)
{
    if (node == &vec->array[0])
        return VECTOR_EOF;
    else
    {
        VectorNode previous = node - 1;

        if (previous->value == NULL)
            previous = &vec->array[vec->capacity / 2 - 1];

        return previous;
    }
}

Pointer vector_node_value(Vector vec, VectorNode node)
{
    return node->value;
}

VectorNode vector_find_node(Vector vec, Pointer value, CompareFunc compare)
{
    // Διάσχιση του vector
    for (int i = 0; i < vec->size; i++)
        if (compare(vec->array[i].value, value) == 0)
            return &vec->array[i]; // βρέθηκε

    return VECTOR_EOF; // δεν υπάρχει
}