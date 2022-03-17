#ifndef LIBSYS_H
#define LIBSYS_H

// Error codes
#define LIB_SUCCESS 0
#define LIB_FILE_ERROR 1
#define LIB_ADD_FAILED 2
#define LIB_REC_NOT_FOUND 3
#define LIB_REPO_ALREADY_OPEN 12
#define LIB_NDX_SAVE_FAILED 13
#define LIB_REPO_NOT_OPEN 14
#define BOOK_SUCCESS 0
#define BOOK_FAILURE 1

#define RECORD_DELETED 0
#define RECORD_PRESENT 1
#define RECORD_NEVER_EXISTED 2

// Repository status values
#define LIB_REPO_OPEN 10
#define LIB_REPO_CLOSED 11
#define LIB_MAX_INDEX_ENTRIES 10000

// Needed constants
#define repo_name_length 30
#define item_name_length 40

struct index_unit
{
	int flag;
	char *key;
	int offset;
};

struct index
{
	FILE *fp;
	char *name;
	struct index_unit index_entries[LIB_MAX_INDEX_ENTRIES];
	int index_count;
	int size;
};

struct linear_repo
{
	char name[repo_name_length];
	FILE *data_fp;
	int repo_status;
	int size;
};

struct indexed_repo
{
	char name[repo_name_length];
	FILE *data_fp;
	int repo_status;
	int index_list_count;
	struct index *index_list;
	int size;
};

struct LibsysInfo
{
	struct indexed_repo book_repo;
	struct indexed_repo stud_repo;
	struct linear_repo issue_repo;
};

struct Book
{
	int isbn;
	char title[item_name_length];
	char author[item_name_length];
	float price;
};

struct Student
{
	int rollno;
	char name[item_name_length];
	char address[item_name_length];
	float cgpa;
};

struct Issue
{
	int rollno;
	int isbn;
};

int get_str_len(int num);

int get_str(int num, char *str);

//--------------------------------create()---------------------------------
int libsys_create(char *book_name, char *stud_name, char *issue_name);

int linear_repo_build(int size, char name[repo_name_length]);

int indexed_repo_build(int size, char name[repo_name_length], int index_list_count, int *key_size_list);

int index_build(int id, int size, char name[repo_name_length]);

//--------------------------------open()---------------------------------
int libsys_open(char *book_name, char *stud_name, char *issue_name);

int indexed_repo_open(struct indexed_repo *rep, char *repo_name);

int index_open(struct index *ndx, char *name, int id);

int linear_repo_open(struct linear_repo *rep, char *repo_name);

//------------------------------book:NO change ----------------------------

int get_book_by_isbn(int key, struct Book *rec);

int put_book_by_isbn(int key, struct Book *rec);

int delete_book_by_isbn(int key);

//-----------------------------student: additional code---------------------------
int put_student_by_rollno(int rollno_to_write, struct Student *rec);

int get_student_by_rollno(int rollno_to_read, struct Student *rec);

int delete_student_by_rollno(int rollno);

//-----------------------------General Repo: Additional Code------------------------
int get_offset_by_id(struct indexed_repo *rep, int index_num, char *key, int size, int *flag, int *index_entry_num);
int put_indexed_record_by_id(struct indexed_repo *rep, int index_num, char *key, int size, char *rec, int index_entry_num);
int get_indexed_record_by_id(struct indexed_repo *rep, int index_num, char *key, int size, char *rec);
int delete_indexed_record_by_id(struct indexed_repo *rep, int index_num, char *key, int size);
int put_linear_record_by_id(struct linear_repo *rep, char *key, int size, char *rec);
int get_linear_record_by_id(struct linear_repo *rep, char *key, int size, char *rec);

//--------------------------------issue()---------------------------------
int issue(int rollno, int isbn);
int get_issue(int isbn, struct Issue *rec);

//----------------------------------close()---------------------------------
int libsys_close();

int indexed_repo_close(struct indexed_repo *rep);

int index_close(struct index *ndx);

int linear_repo_close(struct linear_repo *rep);

//---------------------------------------------------Additional Functions -----------------------
int get_book_by_title(char *title, struct Book *rec);
int get_student_by_name(char *name, struct Student *rec);
int delete_book_by_title(char *title);
int delete_student_by_roll_number(int key);
int delete_student_by_name(char *name);

#endif
