/// # Improved Robinhood Hash Table
/// ## A Proof of Concept
///
///
///

use std::vec::Vec;

#[derive(Debug)]
#[derive(Clone)]	 // Needed for creating vector of these with a None (don't really understand why)
struct TableItem {
	// unrelated to the specific item
	offset: usize,

	// related to the specific item
	hashcode: usize,
	key: u32,	// generic type in general (with trait maybe?)
	value: u32,	// generic type in general (with trait maybe?)
}


#[derive(Debug)]
struct Table {
	table: Vec<Option<TableItem>>,	// Not sure if this is the best data structure
	len: usize,
	cap: usize,
}

/// Hash a u32 value.
/// Taken from [stackoverflow](https://stackoverflow.com/questions/664014/what-integer-hash-function-are-good-that-accepts-an-integer-hash-key)
fn hash_key(key: u32) -> usize {
	let mut hashcode: usize = ((key as usize >> 16) ^ key as usize) * 0x45d9f3b;
	hashcode = ((hashcode >> 16) ^ hashcode) * 0x45d9f3b;
	hashcode = (hashcode >> 16) ^ hashcode;
	hashcode
}

impl Table {
	fn init(cap: usize) -> Self {
		let mut t = Table {		// This mut is supposedly unnecessary. I'm not sure why.
			table: vec![None; cap],
			len: 0,
			cap: cap,
		};
		t
	}

	/// Index or false if not found
	fn get_table_idx(&self, key: u32, hashcode: usize) -> Result<usize, i32> {

		let home: usize = hashcode % self.cap;
		for offset in 0..self.cap {
			let table_idx: usize = (home + offset) % self.cap;
			match self.table[table_idx] {
				None => { return table_idx; },
				Some(x) => {
					if x.hashcode == hashcode && x.key == key {
						return table_idx;
					}
					continue;
				},
			}
		}

		Err(-1)
	}

	/// Returns true if the value was replaced, other
	fn insert(&mut self, key: u32, value: u32) -> Result<bool, i32> {
		if self.len > self.cap {
			panic!("len > cap!");
		} else if self.len == self.cap {
			// let mut new_table = Table {
			// 	table: vec![None; 2 * cap],
			// 	len: 0,
			// 	cap: 2 * cap,
			// };

			// TODO - expand array
			assert_eq!(self.len, self.cap);	// Remove
		}

		let mut item = TableItem {
			offset: 0, hashcode: hash_key(key), key, value,
		};
		let home: usize = item.hashcode % self.cap;
		for offset in 0..self.cap {
			let table_idx: usize = (home + offset) % self.cap;
			println!("{}, {}: {:?}", offset, table_idx, self);
			match self.table[table_idx] {
				None => {
					self.table[table_idx] = Some(item);
					return Ok(false);
				},
				Some(_) => {
					// TODO - check if it matches, if not then continue
				},
			};
		}

		Err(-1)
	}
}

fn main() {
	let mut t = Table::init(10);

	let b: Result<bool, i32> = t.insert(0, 0);
	match b {
		Ok(inner_b) => {println!("Value already in table: {}", inner_b);}, Err(_) => panic!("Insert failed!")
	};

	println!("{:?}", t);	// add "{:#?}" for pretty printing
}
