#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <cstdlib>
const int max_buff = 65536;
const int limit_process = 25;
const int limit_record = 530; // 512 data + 11 others = 523 charaters
const int bytes_per_line = 16;

/* 
  Clear the command, terminal Function
*/
void clear_screen()
{
#ifdef _WIN32
  std::system("cls");
#else
  // Assume POSIX
  std::system("clear");
#endif
}

/* 
Convert a heximal string to decimal integer

Input: heximal string
Output: Decimal Integer corresponding to the string, or -1 if the string is invalid
*/
int convert_decimal(char *hex)
{
  int result = 0;
  int len = strlen(hex);
  for (int i = 0; i < len; ++i)
  {
    int digit = 0;
    if (hex[i] >= '0' && hex[i] <= '9')
    {
      digit = hex[i] - '0';
    }
    else if (hex[i] >= 'a' && hex[i] <= 'f')
    {
      digit = (hex[i] - 'a') + 10;
    }
    else if (hex[i] >= 'A' && hex[i] <= 'F')
    {
      digit = (hex[i] - 'A') + 10;
    }
    else
    {
      return -1;
    }
    result = result * 16 + digit;
  }
  return result;
}

/* 
Convert a decimal integer to 4-digit heximal string (for memory)

Input: decimal integer
Output: Decimal Integer corresponding to the string, or -1 if the string is invalid
*/
char* convert_heximal(int dec)
{
  char* ans = (char*)calloc(4, sizeof(char));
  for (int i = 3; i >= 0; -- i) {
    int divide = 1;
    for (int j = 0; j < i; ++ j) divide *= 16;
    int digit = dec / divide;
    dec %= divide;
    ans[3 - i] = (digit < 10 ? char(digit + '0') : char(digit - 10 + 'A'));
  }
  return ans;
}

/* 
Given 2 strings a and b, check whether b is the suffix of a or not.

Input: 2 strings
Output: Boolean, True if b is the suffix of a and False, otherwise
*/
bool checkSuffix(const char *str, const char *suffix)
{
  if (!str || !suffix)
    return false;
  size_t len_str = strlen(str);
  size_t len_suffix = strlen(suffix);
  if (len_suffix > len_str)
  {
    return false;
  }
  return (strncmp(str + len_str - len_suffix, suffix, len_suffix)) == 0;
}

/* 
Struct type for record
*/
struct record
{
  int address;
  int operation;
  int len_data;
  char **list_data;
  int memory;
  int checksum;
};

/* 
Create a record based on a string

Input: A record pointer and a string without a newline character at the end
Output: Boolean, True if success, False if error occurs
*/
bool create(struct record *target, char *str)
{
  int len = strlen(str);
  // 11 is the minimum character for a record string (1 character ':', 2 charaters for the len_data, 4 characters for the memory, 2 characters for the operation types, 2 chacters for the checksum)
  if (len < 11)
    return false;

  // : charater beginning
  if (str[0] != ':')
  {
    printf("Error: Record must begin with : charater\n");
    return false;
  }
  // len_data
  char *len_data = (char *)calloc(2, sizeof(char));
  for (int i = 0; i < 2; ++i)
  {
    len_data[i] = str[i + 1];
  }
  int len_data_int = convert_decimal(len_data);
  if (len_data_int == -1)
  {
    printf("Error: Invalid len data %s\n", len_data);
    return false;
  }
  target->len_data = len_data_int;

  // memory
  char *memory = (char *)calloc(4, sizeof(char));
  for (int i = 0; i < 4; ++i)
  {
    memory[i] = str[i + 3];
  }
  int memory_int = convert_decimal(memory);
  if (memory_int == -1)
  {
    printf("Error: Invalid Heximal Memory %s\n", memory);
    return false;
  }
  target->memory = memory_int;

  // operation
  char *operation = (char *)calloc(2, sizeof(char));
  for (int i = 0; i < 2; ++i)
  {
    operation[i] = str[i + 7];
  }
  int operation_int = convert_decimal(operation);
  if (operation_int == -1)
  {
    printf("Error: Invalid Heximal Operation: %s\n", operation);
    return false;
  }
  int valid_operation[] = {0, 1, 2, 4, 5};
  bool is_valid = false;
  for (int i = 0; i < 5; ++i)
  {
    if (operation_int == valid_operation[i])
    {
      is_valid = true;
      break;
    }
  }
  if (!is_valid)
  {
    printf("Error: %s is not a valid operation\n", operation);
    return false;
  }
  target->operation = operation_int;

  // compare data length
  if (len - 11 != len_data_int * 2)
  {
    printf("Error: The data length doesn't match with the string\n");
    printf("The data field is %d, but the data length in the string is %d\n", len_data_int * 2, len - 11);
    return false;
  }

  if (operation_int == 2 && len_data_int != 2)
  {
    printf("Error: Extended segment address record must have 2 bytes data\n");
    return false;
  }

  if (operation_int == 4 && len_data_int != 2)
  {
    printf("Error: Extended linear address record must have 2 bytes data\n");
    return false;
  }

  if (operation_int == 5 && len_data_int != 4)
  {
    printf("Error: Start Linear Address Records must have 4 bytes data\n");
    return false;
  }

  target->list_data = (char **)calloc(len_data_int, sizeof(char *));
  for (int i = 0; i < len_data_int; ++i)
  {
    target->list_data[i] = (char *)calloc(2, sizeof(char));
    target->list_data[i][0] = str[2 * i + 9];
    target->list_data[i][1] = str[2 * i + 10];
    int x = convert_decimal(target->list_data[i]);
    if (x == -1)
    {
      printf("Error: Data %s is not valid\n", target->list_data[i]);
      return false;
    }
  }

  // checksum
  char *checksum = (char *)calloc(2, sizeof(char));
  checksum[0] = str[len - 2];
  checksum[1] = str[len - 1];
  int checksum_int = convert_decimal(checksum);
  if (checksum_int == -1)
  {
    printf("Error: Checksum %s is invalid\n", checksum);
    return false;
  }
  // check checksum of the record
  int sum = len_data_int + operation_int + memory_int / 256 + memory_int % 256;
  sum %= 256;
  for (int i = 0; i < len_data_int; ++i)
  {
    int x = convert_decimal(target->list_data[i]);
    printf("%d ", x);
    sum += x;
    sum %= 256;
  }
  printf("\n");
  sum = 256 - sum;
  sum %= 256;
  if (sum != checksum_int)
  {
    printf("Error: Invalid checksum\n");
    printf("Expected: %d, but got %d\n", sum, checksum_int);
    return false;
  }
  target->checksum = checksum_int;
  return true;
}

/*
  Buffer update Function

  Input: record, buffer, range_add(memory add for 4 bits)
  Output: updating buffer
*/

void update_buffer(struct record *target, char **buffer, int &range_add)
{
  switch (target->operation)
  {
  case 5:
  case 1:
    break;
  case 2:
    if (target->list_data[0][0] == '0')
    {
      range_add = -1;
    }
    else
    {
      char *add = (char *)calloc(3, sizeof(char));
      add[0] = target->list_data[0][1];
      add[1] = target->list_data[1][0];
      add[2] = target->list_data[1][1];
      range_add = convert_decimal(add);
    }
    break;
  case 4:
    if (strcmp(target->list_data[0], "00") && strcmp(target->list_data[1], "00"))
    {
      range_add = -1;
    }
    else
    {
      range_add = 0;
    }
    break;
  case 0:
    if (range_add == -1)
    {
      break;
    }
    int assign_memory = target->memory + range_add;
    if (assign_memory >= max_buff)
    {
      break;
    }
    for (int i = 0; i < target->len_data; ++i)
    {
      buffer[assign_memory + i] = target->list_data[i];
    }
  }
}

/* 
  Process Buffer - Process string, creating record and updating buffer
  
  Input: fstream, file_name, buffer data
  Output: Boolean, True if success, False - otherwise 
*/
bool process_buffer(FILE *fp, char *file_name, char **buffer)
{
  // char* str;
  char *str = (char *)calloc(limit_record, sizeof(char));
  int id = 0;
  int range_add = 0;

  while (fgets(str, limit_record, fp))
  {
    int len = strlen(str);
    id++;

    //eliminate newline character
    while (len && (str[len - 1] == '\n' || (int)str[len - 1] == 13)){
      str[len - 1] = '\0';
      len --;
    }

    struct record new_record;

    bool success = create(&new_record, str);
    if (!success)
    {
      printf("Error occurs on record %d\n", id);
      return false;
    }
    update_buffer(&new_record, buffer, range_add);
  }
  return true;
}

/*
  Print Batch Iteratively (25 lines)
*/
void batch_print(char** buffer, bool& working, int& index) {
  for (int i = 0; i < limit_process; ++ i) {
    printf("%s\t", convert_heximal(index));
    char* ascii = (char*)calloc(bytes_per_line + 1, sizeof(char));
    for (int j = 0; j < bytes_per_line; ++ j) {
      printf("%s ", (strcmp(buffer[index], "0") == 0 ? "FF" : buffer[index]));
      int x = convert_decimal(buffer[index]);
      if (strcmp(buffer[index], "0") == 0) ascii[j] = ' ';
      else ascii[j] = (x < 32 ? '.' : char(x));
      index ++;
      if (index >= max_buff) return;
    }
    printf("\t%s\n", ascii);
  }
}

/* 
The root process of the program

Input: File name string
Output: Display the result
*/
void main_process(char *file_name)
{
  FILE *fp;

  char *buffer[max_buff];

  for (int i = 0; i < max_buff; ++ i) {
    buffer[i] = "0";
  }

  // Check file type
  if (!checkSuffix(file_name, ".hex"))
  {
    printf("%s is not Intel Hex File\n", file_name);
    return;
  }

  // // Check the readability of the file
  fp = fopen(file_name, "r");
  if (fp == NULL)
  {
    printf("%s could not be opened\n", file_name);
    return;
  }

  if(process_buffer(fp, file_name, buffer) == false) return;

  bool working = true;
  int index = 0;
  do
  {
    // print the data
    clear_screen();
    batch_print(buffer, working, index);
    if (index >= max_buff) {
      return ;
    }
    // continue command
    if (working)
    {
      printf("Continue?(type \"yes\" to continue): ");
      char s[10];
      scanf("%s", s);
      if (strcmp(s, "yes") != 0)
      {
        working = false;
      }
    }
  } while (working);
}

int main(int argc, char **args)
{
  char *file_name = args[1];

  main_process(file_name);

  return 0;
}