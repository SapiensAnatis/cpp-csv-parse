#include <array>
#include <bitset>
#include <chrono>
#include <fstream>
#include <iostream>
#include <unordered_map>
#include <vector>

#include "immintrin.h"

constexpr size_t num_cols = 7;

template <size_t NCols>
class CsvParseResult
{
public:
  typedef std::array<std::string, NCols> Row;

  CsvParseResult(Row column_names, std::vector<Row> rows)
      : column_names(std::move(column_names)), rows(std::move(rows)) {}

  std::unordered_map<std::string, std::string> get_row(int row_num) const
  {
    std::unordered_map<std::string, std::string> result;
    result.reserve(NCols);

    auto row = rows.at(row_num);

    for (int i = 0; i < NCols; i++)
    {
      result.emplace(column_names[i], row[i]);
    }

    return result;
  }

private:
  Row column_names;
  std::vector<Row> rows;
};

constexpr int CHUNK_SIZE = 256 / 8; // how many chars we can fit in one vector

// Function to display SIMD array data
void display_simd_array(const char *message, __m256i simdArray)
{
  char *data = (char *)&simdArray;
  std::cout << message;
  for (int i = 0; i < CHUNK_SIZE; ++i)
  {
    std::cout << data[i] << ", ";
  }
  std::cout << std::endl;
}

void display_simd_array_numeric(const char *message, __m256i simdArray)
{
  uint8_t *data = (uint8_t *)&simdArray;
  std::cout << message;
  for (int i = 0; i < CHUNK_SIZE; ++i)
  {
    std::cout << static_cast<int>(data[i]) << ", ";
  }
  std::cout << std::endl;
}

void read_col_names(const std::string &row_str,
                    CsvParseResult<num_cols>::Row *output)
{
  size_t pos = 0;
  size_t last = 0;
  size_t idx = 0;
  while ((pos = row_str.find(',', last)) != std::string::npos)
  {
    auto col_name = row_str.substr(last, pos - last);
    output->at(idx) = col_name;
    last = pos + 1;
    idx++;
  }

  output->at(idx) = row_str.substr(last, row_str.size() - 1 - last);
}

void read_row(const std::string &row_str,
              CsvParseResult<num_cols>::Row *output)
{

  bool open_quote = false;
  int processed = 0;

  int col_start = 0;
  int col_num = 0;

#ifdef SIMD // SIMD does not actually make the program observably faster...
  __m256i comma = _mm256_set1_epi8(',');
  __m256i quote = _mm256_set1_epi8('"');

  while (processed < static_cast<int>(row_str.length()) - CHUNK_SIZE)
  {
    const char *ptr = row_str.c_str() + processed;

    __m256i chunk = _mm256_loadu_si256((__m256i_u *)ptr);
    // Vector of 255 where there are commas
    __m256i comma_eq = _mm256_cmpeq_epi8(chunk, comma);
    // Vector of 255 where there are quotes
    __m256i quote_eq = _mm256_cmpeq_epi8(chunk, quote);

    // Reduce equality vectors into 32 bit ints, where bit
    // being 1 = 255 element. Least significant bit = first vector element
    auto comma_int_mask = _mm256_movemask_epi8(comma_eq);
    auto quote_int_mask = _mm256_movemask_epi8(quote_eq);

    while (quote_int_mask || comma_int_mask)
    {
      uint32_t quote_pos = __builtin_ctz(quote_int_mask);
      uint32_t comma_pos = __builtin_ctz(comma_int_mask);

      if (quote_pos < comma_pos)
      {
        open_quote = !open_quote;
        quote_int_mask &= quote_int_mask - 1; // Set least significant bit to 0
      }
      else if (!open_quote)
      {
        auto current_string_pos = processed + comma_pos;
        auto col_value =
            row_str.substr(col_start, current_string_pos - col_start);
        output->at(col_num) = col_value;

        col_start = current_string_pos + 1;
        col_num++;

        comma_int_mask &= comma_int_mask - 1;
      }
      else
      {
        break;
      }
    }

    processed += CHUNK_SIZE;
  }
#endif

  // Scalar fallback
  for (int i = processed + 1; i < row_str.size(); i++)
  {
    char ch = row_str[i];

    if (ch == '"')
    {
      open_quote = !open_quote;
    }
    else if (ch == ',' && !open_quote)
    {
      auto col_value = row_str.substr(col_start, i - col_start);
      output->at(col_num) = col_value;

      col_start = i + 1;
      col_num++;
    }
  }

  output->at(col_num) =
      row_str.substr(col_start, row_str.size() - 1 - col_start);
}

int main()
{

#ifdef SIMD
  std::cout << "Using SIMD for parsing" << std::endl;
#endif

  // a
  auto start = std::chrono::high_resolution_clock::now();

  std::ifstream input_file("annual-enterprise-survey-2023-financial-year-"
                           "provisional-size-bands.csv");

  if (!input_file.is_open())
  {
    std::cout << "Failed to open file" << std::endl;
    return 1;
  }

  std::string line;
  std::getline(input_file, line);

  CsvParseResult<num_cols>::Row column_names{};
  std::vector<CsvParseResult<num_cols>::Row> rows{};
  rows.reserve(20000);

  read_col_names(line, &column_names);
  CsvParseResult<num_cols>::Row row;

  while (std::getline(input_file, line))
  {
    read_row(line, &row);
    rows.push_back(row);
  }

  CsvParseResult<num_cols> result(std::move(column_names), std::move(rows));

  auto end = std::chrono::high_resolution_clock::now();

  for (auto pair : result.get_row(1))
  {
    std::cout << pair.first << ": " << pair.second << "\n";
  }

  std::chrono::duration<float> elapsed_ms =
      std::chrono::duration_cast<std::chrono::duration<float>>(end - start);

  std::cout << "Execution Time = " << elapsed_ms.count() << "s" << std::endl;

  input_file.close();
  return 0;
}