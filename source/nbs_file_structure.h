#pragma once
#include <iostream>
#include <fstream>
#include <vector>
#include "type_read.h"		//需要此文件控制读取

class nbs_head;				//文件头
class nbs_note;				//音符：最小单位
class nbs_track;			//轨道：音符的集合+附属信息
class nbs_file;				//文件：轨道的集合+文件头



class nbs_head{				//没啥用，为了读而读
public:
	int version_check;		//支持的版本此项为0
	int nbs_version;
	int nbs_vanilla_instrument_count;
	int nbs_length;
	int layer_count;
	string nbs_song_info[5];//无用
	int nbs_song_stats[12]; //无用
	nbs_head() { version_check = -1; }
	nbs_head(ifstream& infile);
};

//构造函数直接完成文件读入
nbs_head::nbs_head(ifstream& infile) {
	version_check = type_read(infile, Short);
	if (version_check != 0) return;//不处理老版nbs

	nbs_version = type_read(infile, Byte);
	nbs_vanilla_instrument_count = type_read(infile, Byte);
	nbs_length = type_read(infile, Short);
	layer_count = type_read(infile, Short);

	//以下是无关紧要的文件头内容，不用看
	for (int i = 0; i < 4; i++) type_read(infile, String, &nbs_song_info[i]);
	for (int i = 0; i < 9; i++) type_read(infile, nbs_head_seq[i + 9], &nbs_song_stats[i]);
	type_read(infile, String, &nbs_song_info[4]);
	for (int i = 9; i < 12; i++) type_read(infile, nbs_head_seq[i + 10], &nbs_song_stats[i]);
}



class nbs_note {
public:
	int tick, track, inst, key, vol, pan;	//横纵位置，乐器，音高，音量，声像
	nbs_note() { tick = -1; track = -1; inst = -1; key = -1; vol = 0; pan = 0; }
	nbs_note(int t, int tr, int i, int k, int vo, int p) { tick = t; track = tr; inst = i; key = k; vol = vo; pan = p; }
	nbs_note(nbs_note& n) {
		tick = n.tick;
		track = n.track;
		inst = n.inst;
		key = n.key;
		vol = n.vol;
		pan = n.pan;
	}
	void refresh(ifstream& infile, int t, int tr) { //仅读取nbs文件时使用
		tick = t;
		track = tr;
		inst = type_read(infile, Byte);
		key = type_read(infile, Byte);
		vol = type_read(infile, Byte);
		pan = type_read(infile, Byte);
		//pit = type_read(infile, Short);
	}
};



//最重要的类，因为大多数操作对轨道进行
class nbs_track {
public:
	static int track_count;				//用于创建nbs文件类时自动编号
	string layer_name;
	int no;
	bool locked;
	int nbs_volume;
	int nbs_stereo;
	int inst;							//0-15，默认每个音轨只有一种乐器（良好的编曲习惯），由第一个音符决定，空音轨乐器为-1
	int track_length;					//最后一个音的tick位置，不等于音符数目！
	vector<nbs_note*> notes;			//notes不被初始化，所有track的音符信息初始均为空，之后用copy_notes拷贝

	nbs_track(nbs_track& t) {			//拷贝或继承用这个，不得使用默认构造函数；不继承音符信息
		layer_name = t.layer_name;
		no = t.no;
		locked = t.locked;
		nbs_volume = t.nbs_volume;
		nbs_stereo = t.nbs_stereo;
		inst = t.inst;
		track_length = -1;
	}
	nbs_track(string name, int num) {	//创建独立轨道用这个，不得使用默认构造函数
		layer_name = name;
		no = num;
		locked = true;
		nbs_volume = 0;
		nbs_stereo = 0;
		inst = -1;
		track_length = -1;
	}
	nbs_track() {						//仅供nbs文件类初始化使用，自动编号
		layer_name = string("Unnamed");
		no = track_count++;
		locked = true;
		nbs_volume = 0;
		nbs_stereo = 0;
		inst = -1;
		track_length = -1;
	}

	void copy_notes(nbs_track& t) {
		notes = t.notes;
		track_length = t.track_length;
	}
	
	void refresh(ifstream& infile) {	//仅读取nbs文件时使用
		if (!type_read(infile, String, &layer_name)) {
			layer_name = string("Layer ") + to_string(no + 1);
		}
		type_read(infile, Byte, (int*)&locked);
		type_read(infile, Byte, &nbs_volume);
		type_read(infile, Byte, &nbs_stereo);
		if (track_length >= 0)
			inst = notes[0]->inst;
	}

	~nbs_track() {
		if (notes.size() != 0) {
			for (int i = notes.size() - 1; i >= 0; i--)
				if (notes[i] != NULL)
					delete notes[i];
			notes.clear();
		}
	}
};

int nbs_track::track_count = 0;



class nbs_file {
public:
	nbs_head head;
	nbs_track track[64];
	int num_of_track;

	nbs_file() { nbs_track::track_count = 0; head.version_check = -1; };
	nbs_file(ifstream& infile);

	void construct_nbs_track(ifstream& infile);
	void read_track_info(ifstream& infile);

	~nbs_file() {
		nbs_track::track_count = 0;
		for (int i = 0; i < head.layer_count; i++) 
			track[i].~nbs_track();
	}
};

//track附属信息，官网说明part3
void nbs_file::read_track_info(ifstream& infile) {
	for (int i = 0; i < head.layer_count; i++) {
		track[i].refresh(infile);
	}
}

//track音轨信息，官网说明part2
void nbs_file::construct_nbs_track(ifstream& infile) {
	//nbs文件记录每个音符到上一个音符的“距离”，即横向时间距离，纵向音轨距离
	//读取时，从上到下先读完每个tick所有音轨，再从左到右读完所有tick
	//该函数从给定文件指针（已定位到记录音轨的部分），读取音符事件到track数组

	int h_pos = -1, v_pos = -1, h_jump, v_jump;
	bool tick_changed = false;			//记录是否发生列（时间）跳转，用于判断音轨结束

	// h_jump, v_jump:（到下一个音的）水平时间跳转距离，垂直音轨跳转距离
	// h_pos, v_pos: 当前时间位置，当前音轨位置

						
	do {								//读到水平0，垂直0代表列结束；水平0，垂直0，水平0代表所有音轨结束
		type_read(infile, Short, &h_jump);
		if (h_jump == 0)
			if (tick_changed) return;	//所有音轨结束
		h_pos += h_jump;
		type_read(infile, Short, &v_jump);
		if (v_jump == 0) {				//列结束，跳转tick
			v_pos = -1;
			tick_changed = true;
		}
		else {
			tick_changed = false;
			v_pos += v_jump;
			//找到了一个音符
			nbs_note* new_note = new(nbs_note);
			new_note->refresh(infile, h_pos, v_pos);
			track[v_pos].notes.push_back(new_note);
			track[v_pos].track_length= h_pos;
		}
	} while (true);
}

nbs_file::nbs_file(ifstream& infile):head(infile), track() {	//head构造函数读取part1
	nbs_track::track_count = 0;
	if (head.version_check != 0) return;						//不支持老版nbs格式
	num_of_track = head.layer_count;
	construct_nbs_track(infile);								//读取part2
	read_track_info(infile);									//读取part3（part4忽略）
}

