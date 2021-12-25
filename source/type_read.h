#pragma once
#include <iostream>
#include <fstream>
#include <string>

/*
为了方便读取.nbs文件中不同类型的数据编写了type_read函数，数据类型定义在enum_read_type，文件结构在官网opennbs.org/nbs
用法：
1. 变量=type_read（文件，类型）
2. type_read（文件，类型，变量指针int*或string*）
*/

enum enum_read_type { Byte, Short, Int, String };
int debug_pointer_location;
enum_read_type nbs_head_seq[22] = { Short,Byte,Byte,Short,Short,String,String,String,String,Short,Byte,Byte,Byte,Int,Int,Int,Int,Int,String,Byte,Byte,Short };
enum_read_type nbs_track_seq[4] = { String, Byte, Byte, Byte };
enum_read_type nbs_note_seq[7] = { Short,Short,Byte,Byte,Byte,Byte,Short };

void type_read(ifstream& infile, enum_read_type type, int* buf) {
	char temp[5];
	if (type == Short) {
		infile.read(temp, 2);
		*buf = int((temp[1] << 8) + temp[0]);
		debug_pointer_location += 2;
	}
	else if (type == Int) {
		infile.read(temp, 4);
		*buf = int((temp[3] << 24) + (temp[2] << 16) + (temp[1] << 8) + temp[0]);
		debug_pointer_location += 4;
	}
	else if (type == Byte) {
		*buf = int(infile.get());
		debug_pointer_location += 1;
	}
}

int type_read(ifstream& infile, enum_read_type type) {
	char temp[5]; int buf;
	if (type == Short) {
		infile.read(temp, 2);
		debug_pointer_location += 2;
		return buf = int((temp[1] << 8) + temp[0]);
	}
	else if (type == Int) {
		infile.read(temp, 4);
		debug_pointer_location += 4;
		return buf = int((temp[3] << 24) + (temp[2] << 16) + (temp[1] << 8) + temp[0]);
	}
	else if (type == Byte) {
		debug_pointer_location += 1;
		return buf = int(infile.get());
	}
}

int type_read(ifstream& infile, enum_read_type type, string* buf) {
	char temp[257];
	int str_len = type_read(infile, Int);
	infile.read(temp, str_len);
	buf->assign(temp, 0, str_len);
	debug_pointer_location += str_len;
	return str_len;
}