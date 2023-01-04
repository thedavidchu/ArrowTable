#include "arrowtable.h"

int main(void) {
	struct Table table;
	create(&table);

	insert(&table, "hello", 10);
	// insert(&table, "hello 1", 11);
	// insert(&table, "hello 2", 12);
	// insert(&table, "hello 3", 13);
	// insert(&table, "hello 4", 14);
	// insert(&table, "hello 5", 15);
	// insert(&table, "hello 6", 16);
	// insert(&table, "hello 7", 17);
	insert(&table, "hello 8", 18);
	insert(&table, "hello 9", 19);
	insert(&table, "hello", 20);
	insert(&table, "hello ;", 21);
	insert(&table, "hello :", 22);
	insert(&table, "hello <", 23);
	insert(&table, "good-bye 0", 23);

	print(&table);

	destroy(&table);
	return 0;
}