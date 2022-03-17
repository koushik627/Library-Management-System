#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include "libsys.h"
#define LFE_Check(temp)        \
	if (temp)                  \
	{                          \
		return LIB_FILE_ERROR; \
	}
#define GE_Check(temp)       \
	if (temp != LIB_SUCCESS) \
	{                        \
		return temp;         \
	}

// Define the global variable of libsys
struct LibsysInfo repo_handle;

int get_str_len(int num)
{
	if (num == 0)
	{
		return 1;
	}
	int i;
	for (int i = 0; num > 0; i++)
	{
		num /= 10;
	}
	return i;
}

int get_str(int num, char *str)
{
	if (num == 0)
	{
		strcpy(str, "0");
	}
	int num2 = 0;
	int trailting_zeroes = 0;
	while (num > 0)
	{
		num2 = num2 * 10 + num % 10;
		if (num % 10 == 0 && num2 == 0)
		{
			trailting_zeroes++;
		}
		num /= 10;
	}
	int i;
	for (i = 0; num2 > 0; i++)
	{
		str[i] = '0' + num2 % 10;
		num2 /= 10;
	}
	for (int c = 0; c < trailting_zeroes; c++)
	{
		str[i] = '0';
		i++;
	}
	return i;
}

//--------------------------------create()---------------------------------
int libsys_create(char *book_name, char *stud_name, char *issue_name)
{
	int temp[2] = {sizeof(int), item_name_length};
	GE_Check(indexed_repo_build(sizeof(struct Book), book_name, 2, temp));
	GE_Check(indexed_repo_build(sizeof(struct Student), stud_name, 2, temp));
	GE_Check(linear_repo_build(sizeof(struct Issue), issue_name));
	return LIB_SUCCESS;
}

int linear_repo_build(int size, char name[repo_name_length])
{
	struct linear_repo rep;
	char name_data[repo_name_length + 4];
	strcpy(name_data, name);
	strcat(name_data, ".dat");
	rep.data_fp = fopen(name_data, "wb");
	LFE_Check(rep.data_fp == NULL);
	LFE_Check(fwrite(&size, sizeof(size), 1, rep.data_fp) != 1);
	LFE_Check(fclose(rep.data_fp));
	return LIB_SUCCESS;
}

int indexed_repo_build(int size, char name[repo_name_length], int index_list_count, int *key_size_list)
{
	struct indexed_repo rep;
	char name_data[repo_name_length + 4];
	strcpy(name_data, name);
	strcat(name_data, ".dat");
	rep.data_fp = fopen(name_data, "wb");
	LFE_Check(rep.data_fp == NULL);
	rep.size = size;
	rep.index_list_count = index_list_count;
	LFE_Check(fwrite(&rep.size, sizeof(rep.size), 1, rep.data_fp) != 1);
	LFE_Check(fwrite(&rep.index_list_count, sizeof(rep.index_list_count), 1, rep.data_fp) != 1);
	for (int i = 0; i < index_list_count; i++)
	{
		GE_Check(index_build(i, key_size_list[i], name));
	}
	LFE_Check(fclose(rep.data_fp));
	return LIB_SUCCESS;
}

int index_build(int id, int size, char name[repo_name_length])
{
	struct index ndx;
	int id_digits = get_str_len(id);
	char id_str[id_digits + 1];
	get_str(id, id_str);
	id_str[id_digits] = '\0';
	char name_ndx[repo_name_length + 4 + id_digits];
	strcpy(name_ndx, name);
	strcat(name_ndx, id_str);
	strcat(name_ndx, ".ndx");
	ndx.fp = fopen(name_ndx, "wb");
	LFE_Check(ndx.fp == NULL);
	ndx.index_count = 0;
	ndx.size = size;
	LFE_Check(fwrite(&ndx.size, sizeof(ndx.size), 1, ndx.fp) != 1);
	LFE_Check(fwrite(&ndx.index_count, sizeof(ndx.index_count), 1, ndx.fp) != 1);
	LFE_Check(fclose(ndx.fp));
	return LIB_SUCCESS;
}

//--------------------------------open()---------------------------------
int libsys_open(char *book_name, char *stud_name, char *issue_name)
{
	int temp;
	GE_Check(indexed_repo_open(&repo_handle.book_repo, book_name));
	GE_Check(indexed_repo_open(&repo_handle.stud_repo, stud_name));
	GE_Check(linear_repo_open(&repo_handle.issue_repo, issue_name));
	return LIB_SUCCESS;
}

int indexed_repo_open(struct indexed_repo *rep, char *repo_name)
{
	if (rep->repo_status == LIB_REPO_OPEN)
	{
		return LIB_REPO_ALREADY_OPEN;
	}
	strcpy(rep->name, repo_name);
	char name_data[repo_name_length + 4];
	strcpy(name_data, repo_name);
	strcat(name_data, ".dat");
	rep->data_fp = fopen(name_data, "rb+");
	LFE_Check(rep->data_fp == NULL);
	LFE_Check(fread(&rep->size, sizeof(rep->size), 1, rep->data_fp) != 1);
	LFE_Check(fread(&rep->index_list_count, sizeof(rep->index_list_count), 1, rep->data_fp) != 1);
	rep->index_list = malloc((rep->index_list_count) * sizeof(struct index));
	LFE_Check(rep->index_list == NULL);
	for (int i = 0; i < rep->index_list_count; i++)
	{
		GE_Check(index_open(rep->index_list + i, repo_name, i));
	}
	rep->repo_status = LIB_REPO_OPEN;
	return LIB_SUCCESS;
}

int index_open(struct index *ndx, char *name, int id)
{
	int id_digits = get_str_len(id);
	char id_str[id_digits + 1];
	get_str(id, id_str);
	id_str[id_digits] = '\0';
	char name_ndx[repo_name_length + 4 + id_digits];
	strcpy(name_ndx, name);
	strcat(name_ndx, id_str);
	strcat(name_ndx, ".ndx");
	ndx->name = malloc(strlen(name_ndx));
	LFE_Check(ndx->name == NULL);
	strcpy(ndx->name, name_ndx);
	ndx->fp = fopen(name_ndx, "rb+");
	LFE_Check(ndx->fp == NULL);
	LFE_Check(fread(&ndx->size, sizeof(ndx->size), 1, ndx->fp) != 1);
	LFE_Check(fread(&ndx->index_count, sizeof(ndx->index_count), 1, ndx->fp) != 1);
	for (int i = 0; i < ndx->index_count; i++)
	{
		LFE_Check(fread(&ndx->index_entries[i].flag, sizeof(ndx->index_entries[i].flag), 1, ndx->fp) != 1);
		ndx->index_entries[i].key = malloc(ndx->size);
		LFE_Check(ndx->index_entries[i].key == NULL);
		if (ndx->index_entries[i].flag != RECORD_PRESENT)
		{
			LFE_Check(fseek(ndx->fp, ndx->size + sizeof(ndx->index_entries[0].offset), SEEK_CUR));
		}
		else
		{
			LFE_Check(fread(&ndx->index_entries[i].key, ndx->size, 1, ndx->fp) != 1);
			LFE_Check(fread(&ndx->index_entries[i].offset, sizeof(ndx->index_entries[i].offset), 1, ndx->fp) != 1);
		}
	}
	LFE_Check(fclose(ndx->fp));
	return LIB_SUCCESS;
}

int linear_repo_open(struct linear_repo *rep, char *repo_name)
{
	if (rep->repo_status == LIB_REPO_OPEN)
	{
		return LIB_REPO_ALREADY_OPEN;
	}
	strcpy(rep->name, repo_name);
	char data_file_name[repo_name_length + 4];
	strcpy(data_file_name, repo_name);
	strcat(data_file_name, ".dat");
	rep->data_fp = fopen(data_file_name, "rb+");
	LFE_Check(rep->data_fp == NULL);
	LFE_Check(fread(&rep->size, sizeof(rep->size), 1, rep->data_fp) != 1);
	rep->repo_status = LIB_REPO_OPEN;
	return LIB_SUCCESS;
}

//------------------------------book:NO change ----------------------------

int get_book_by_isbn(int key, struct Book *rec)
{
	GE_Check(get_indexed_record_by_id(&repo_handle.book_repo, 0, (char *)&key, 0, (char *)rec));
	return LIB_SUCCESS;
}

int put_book_by_isbn(int key, struct Book *rec)
{
	GE_Check(put_indexed_record_by_id(&repo_handle.book_repo, 0, (char *)&key, 0, (char *)rec, -1));
	int flag, index_entry_num;
	int offset = get_offset_by_id(&repo_handle.book_repo, 0, (char *)&key, 0, &flag, &index_entry_num);
	GE_Check(put_indexed_record_by_id(&repo_handle.book_repo, 1, rec->title, strlen(rec->title), (char *)rec, index_entry_num));
	return LIB_SUCCESS;
}

int delete_book_by_isbn(int key)
{
	struct Issue temp;
	if (get_linear_record_by_id(&repo_handle.issue_repo, (char *)&key, sizeof(key), (char *)&temp) == LIB_SUCCESS)
	{
		return BOOK_FAILURE;
	}
	GE_Check(delete_indexed_record_by_id(&repo_handle.book_repo, 0, (char *)&key, 0));
	return LIB_SUCCESS;
}

int get_book_by_title(char *title, struct Book *rec)
{
	GE_Check(get_indexed_record_by_id(&repo_handle.book_repo, 1, title, strlen(title), (char *)rec));
	return LIB_SUCCESS;
}

int delete_book_by_title(char *title)
{
	struct Book temp;
	get_book_by_title(title, &temp);
	if (get_linear_record_by_id(&repo_handle.issue_repo, (char *)&temp.isbn, sizeof(temp.isbn), (char *)&temp) == LIB_SUCCESS)
	{
		return BOOK_FAILURE;
	}
	GE_Check(delete_indexed_record_by_id(&repo_handle.book_repo, 1, title, strlen(title)));
	return LIB_SUCCESS;
}

//-----------------------------student: additional code---------------------------
int put_student_by_rollno(int rollno_to_write, struct Student *rec)
{
	GE_Check(put_indexed_record_by_id(&repo_handle.stud_repo, 0, (char *)&rollno_to_write, 0, (char *)rec, -1));
	int flag, index_entry_num;
	int offset = get_offset_by_id(&repo_handle.stud_repo, 0, (char *)&rollno_to_write, 0, &flag, &index_entry_num);
	GE_Check(put_indexed_record_by_id(&repo_handle.stud_repo, 1, rec->name, strlen(rec->name), (char *)rec, index_entry_num));
	return LIB_SUCCESS;
}

int get_student_by_rollno(int rollno_to_read, struct Student *rec)
{
	GE_Check(get_indexed_record_by_id(&repo_handle.stud_repo, 0, (char *)&rollno_to_read, 0, (char *)rec));
	return LIB_SUCCESS;
}

int get_student_by_name(char *name, struct Student *rec)
{
	GE_Check(get_indexed_record_by_id(&repo_handle.stud_repo, 1, name, strlen(name), (char *)rec));
	return LIB_SUCCESS;
}

int delete_student_by_rollno(int rollno_to_read)
{
	GE_Check(delete_indexed_record_by_id(&repo_handle.stud_repo, 0, (char *)&rollno_to_read, 0));
	return LIB_SUCCESS;
}

int delete_student_by_name(char *name)
{
	GE_Check(delete_indexed_record_by_id(&repo_handle.stud_repo, 1, name, strlen(name)));
	return LIB_SUCCESS;
}

//-----------------------------Indexed Repo: Additional Code------------------------
int get_offset_by_id(struct indexed_repo *rep, int index_num, char *key, int size, int *flag, int *index_entry_num)
{
	if (rep->repo_status != LIB_REPO_OPEN)
	{
		return -LIB_REPO_NOT_OPEN;
	}
	struct index *ndx = rep->index_list + index_num;
	if (size == 0)
	{
		size = ndx->size;
	}
	for (int i = 0; i < ndx->index_count; i++)
	{
		char temp = 1;
		for (int j = 0; j < size; j++)
		{
			if (key[j] != (ndx->index_entries[i]).key[j])
			{
				temp = 0;
				break;
			}
		}
		if (temp)
		{
			*flag = (ndx->index_entries[i]).flag;
			*index_entry_num = i;
			return ndx->index_entries[i].offset;
		}
	}
	return -LIB_REC_NOT_FOUND;
}

int put_indexed_record_by_id(struct indexed_repo *rep, int index_num, char *key, int size, char *rec, int index_entry_num)
{
	struct Book *temp = (struct Book *)rec;
	if (rep->repo_status != LIB_REPO_OPEN)
	{
		return LIB_REPO_NOT_OPEN;
	}
	int deleted_flag = RECORD_NEVER_EXISTED;
	int primary_key_flag = 1;
	if (index_entry_num < 0)
	{
		int offset = get_offset_by_id(rep, 0, key, size, &deleted_flag, &index_entry_num);
		if (deleted_flag == RECORD_PRESENT)
		{
			return BOOK_FAILURE;
		}
		if (deleted_flag == RECORD_NEVER_EXISTED)
		{
			LFE_Check(fseek(rep->data_fp, 0, SEEK_END));
			index_entry_num = rep->index_list[0].index_count;
			rep->index_list[0].index_entries[index_entry_num].key = malloc(rep->index_list[0].size);
			LFE_Check(rep->index_list[0].index_entries[index_entry_num].key == NULL);
			rep->index_list[0].index_count++;
		}
		else
		{
			LFE_Check(fseek(rep->data_fp, offset, SEEK_SET));
		}
	}
	else
	{
		primary_key_flag = 0;
		if (rep->index_list[index_num].index_count == index_entry_num)
		{
			rep->index_list[index_num].index_entries[index_entry_num].key = malloc(rep->index_list[index_num].size);
			LFE_Check(rep->index_list[index_num].index_entries[index_entry_num].key == NULL);
			rep->index_list[index_num].index_count++;
		}
	}
	rep->index_list[index_num].index_entries[index_entry_num].flag = RECORD_PRESENT;
	for (int i = 0; i < rep->index_list[index_num].size; i++)
	{
		rep->index_list[index_num].index_entries[index_entry_num].key[i] = key[i];
	}
	if (primary_key_flag)
	{
		rep->index_list[0].index_entries[index_entry_num].offset = ftell(rep->data_fp);
		LFE_Check(fwrite(rep->index_list[0].index_entries[index_entry_num].key, 1, rep->index_list[0].size, rep->data_fp) != rep->index_list[0].size);
		LFE_Check(fwrite(rec, rep->size, 1, rep->data_fp) != 1);
	}
	else
	{
		rep->index_list[index_num].index_entries[index_entry_num].offset = rep->index_list[0].index_entries[index_entry_num].offset;
	}
	return LIB_SUCCESS;
}

int get_indexed_record_by_id(struct indexed_repo *rep, int index_num, char *key, int size, char *rec)
{
	if (rep->repo_status != LIB_REPO_OPEN)
	{
		return LIB_REPO_NOT_OPEN;
	}
	int flag, index_entry_num;
	int offset = get_offset_by_id(rep, index_num, key, size, &flag, &index_entry_num);
	if (flag == RECORD_PRESENT)
	{
		LFE_Check(fseek(rep->data_fp, offset, SEEK_SET));
		LFE_Check(fseek(rep->data_fp, rep->index_list[0].size, SEEK_CUR));
		LFE_Check(fread(rec, rep->size, 1, rep->data_fp) != 1);
		return LIB_SUCCESS;
	}
	else
	{
		return LIB_REC_NOT_FOUND;
	}
}

int delete_indexed_record_by_id(struct indexed_repo *rep, int index_num, char *key, int size)
{
	if (rep->repo_status != LIB_REPO_OPEN)
	{
		return LIB_REPO_NOT_OPEN;
	}
	int flag, index_entry_num;
	int offset = get_offset_by_id(rep, index_num, key, size, &flag, &index_entry_num);
	if (flag == RECORD_PRESENT)
	{
		for (int i = 0; i < rep->index_list_count; i++)
		{
			rep->index_list[i].index_entries[index_entry_num].flag = RECORD_DELETED;
		}
		offset = get_offset_by_id(rep, index_num, key, size, &flag, &index_entry_num);
		return LIB_SUCCESS;
	}
	else
	{
		return LIB_REC_NOT_FOUND;
	}
}

//--------------------------------issue()---------------------------------
int issue(int rollno, int isbn)
{
	int temp, flag, index_entry_num;
	temp = get_offset_by_id(&repo_handle.book_repo, 0, (char *)&isbn, 0, &flag, &index_entry_num);
	if (temp < 0)
	{
		return -temp;
	}
	temp = get_offset_by_id(&repo_handle.stud_repo, 0, (char *)&rollno, 0, &flag, &index_entry_num);
	if (temp < 0)
	{
		return -temp;
	}
	struct Issue rec;
	rec.isbn = isbn;
	rec.rollno = rollno;
	GE_Check(put_linear_record_by_id(&repo_handle.issue_repo, (char *)&isbn, sizeof(isbn), (char *)&rec));
	return LIB_SUCCESS;
}

int get_issue(int isbn, struct Issue *rec)
{
	GE_Check(get_linear_record_by_id(&repo_handle.issue_repo, (char *)&isbn, sizeof(isbn), (char *)rec));
	return LIB_SUCCESS;
}

int put_linear_record_by_id(struct linear_repo *rep, char *key, int size, char *rec)
{
	if (rep->repo_status != LIB_REPO_OPEN)
	{
		return LIB_REPO_NOT_OPEN;
	}
	int err = get_linear_record_by_id(rep, key, size, rec);
	if (err == LIB_SUCCESS)
	{
		return BOOK_FAILURE;
	}
	else if (err != LIB_REC_NOT_FOUND)
	{
		return err;
	}
	LFE_Check(fseek(rep->data_fp, 0, SEEK_END));
	LFE_Check(fwrite(key, size, 1, rep->data_fp) != 1);
	LFE_Check(fwrite(rec, rep->size, 1, rep->data_fp) != 1);
	return LIB_SUCCESS;
}

int get_linear_record_by_id(struct linear_repo *rep, char *key, int size, char *rec)
{
	LFE_Check(fseek(rep->data_fp, 0, SEEK_END));
	int size_of_file = ftell(rep->data_fp);
	LFE_Check(size_of_file == -1);
	LFE_Check(fseek(rep->data_fp, sizeof(rep->size), SEEK_SET));
	char check[size];
	int offset = sizeof(rep->size);
	LFE_Check(offset == -1);
	while (offset < size_of_file)
	{
		LFE_Check(fread(check, size, 1, rep->data_fp) != 1);
		offset += size;
		int flag = 1;
		for (int i = 0; i < size; i++)
		{
			if (check[i] != key[i])
			{
				flag = 0;
				break;
			}
		}
		if (flag)
		{
			LFE_Check(fread(rec, rep->size, 1, rep->data_fp) != 1);
			return LIB_SUCCESS;
		}
		else
		{
			LFE_Check(fseek(rep->data_fp, rep->size, SEEK_CUR));
		}
		offset += rep->size;
	}
	return LIB_REC_NOT_FOUND;
}

//----------------------------------close()---------------------------------
int libsys_close()
{
	GE_Check(indexed_repo_close(&repo_handle.book_repo));
	GE_Check(indexed_repo_close(&repo_handle.stud_repo));
	GE_Check(linear_repo_close(&repo_handle.issue_repo));
	return LIB_SUCCESS;
}

int indexed_repo_close(struct indexed_repo *rep)
{
	if (rep->repo_status != LIB_REPO_OPEN)
	{
		return LIB_REPO_NOT_OPEN;
	}
	for (int i = 0; i < rep->index_list_count; i++)
	{
		GE_Check(index_close(rep->index_list + i));
	}
	free(rep->index_list);
	rep->index_list = NULL;
	strcpy(rep->name, "");
	rep->repo_status = LIB_REPO_CLOSED;
	rep->size = 0;
	rep->index_list_count = 0;
	LFE_Check(fclose(rep->data_fp));
	return LIB_SUCCESS;
}

int index_close(struct index *ndx)
{
	ndx->fp = fopen(ndx->name, "wb");
	LFE_Check(ndx->fp == NULL);
	LFE_Check(fseek(ndx->fp, 0, SEEK_SET));
	LFE_Check(fwrite(&ndx->size, sizeof(ndx->size), 1, ndx->fp) != 1);
	LFE_Check(fwrite(&ndx->index_count, sizeof(ndx->index_count), 1, ndx->fp) != 1);
	for (int i = 0; i < ndx->index_count; i++)
	{
		LFE_Check(fwrite(ndx->index_entries + i, 1, ndx->size, ndx->fp) != ndx->size);
		free(ndx->index_entries[i].key);
	}
	free(ndx->name);
	ndx->name = NULL;
	LFE_Check(fclose(ndx->fp));
	return LIB_SUCCESS;
}

int linear_repo_close(struct linear_repo *rep)
{
	if (rep->repo_status != LIB_REPO_OPEN)
	{
		return LIB_REPO_NOT_OPEN;
	}
	strcpy(rep->name, "");
	LFE_Check(fclose(rep->data_fp));
	rep->size = 0;
	rep->repo_status = LIB_REPO_CLOSED;
	return LIB_SUCCESS;
}