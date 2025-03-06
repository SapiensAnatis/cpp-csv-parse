use std::collections::HashMap;
use std::fs::File;
use std::io::{BufRead, BufReader};

struct CsvParseResult<const NCOLUMNS: usize> {
    column_names: [String; NCOLUMNS],
    rows: Vec<[String; NCOLUMNS]>,
}

impl<const COLUMNS: usize> CsvParseResult<COLUMNS> {
    pub fn new(column_names: [String; COLUMNS], rows: Vec<[String; COLUMNS]>) -> Self {
        Self { column_names, rows }
    }

    pub fn get_row(&self, row_num: usize) -> HashMap<&str, &str> {
        let row = &self.rows[row_num];

        row.iter().enumerate().map(|(idx, val)| {
            (self.column_names[idx].as_str(), val.as_str())
        }).collect()
    }

    pub fn read_row(row: &str) -> [String; COLUMNS] {
        let mut result: [String; COLUMNS] = [const { String::new() }; COLUMNS];
        let row_bytes = row.as_bytes();

        let mut open_quote = false;

        let mut col_start = 0;
        let mut col_num = 0;

        for i in 0..row_bytes.len() {
            let char = row_bytes[i];

            if char == b'"' {
                open_quote = !open_quote;
            } else if char == b',' && !open_quote {
                let col_value = &row[col_start..i];
                result[col_num] = String::from(col_value);

                col_start = i + 1;
                col_num += 1;
            }
        }

        result[col_num] = row[col_start..].to_string();

        result
    }
}

const NUM_COLUMNS: usize = 7;

fn main() {
    use std::time::Instant;
    let start = Instant::now();

    let file = File::open("../annual-enterprise-survey-2023-financial-year-provisional-size-bands.csv").unwrap();
    let mut lines_iter = BufReader::new(&file).lines();

    let col_names = CsvParseResult::<NUM_COLUMNS>::read_row(&lines_iter.next().unwrap().unwrap());

    let rows= lines_iter
        .flatten()
        .map(|s| { CsvParseResult::read_row(&s) } )
        .collect::<Vec<[String; NUM_COLUMNS]>>();

    let result = CsvParseResult::new(col_names, rows);

    for (col_name, col_value) in result.get_row(1) {
        println!("{col_name}: {col_value}");
    }

    let elapsed = start.elapsed();
    println!("Elapsed: {:?}", elapsed.as_secs_f32());

}