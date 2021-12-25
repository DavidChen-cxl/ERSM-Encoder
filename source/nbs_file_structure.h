#pragma once
#include <iostream>
#include <fstream>
#include <vector>
#include "type_read.h"		//��Ҫ���ļ����ƶ�ȡ

class nbs_head;				//�ļ�ͷ
class nbs_note;				//��������С��λ
class nbs_track;			//����������ļ���+������Ϣ
class nbs_file;				//�ļ�������ļ���+�ļ�ͷ



class nbs_head{				//ûɶ�ã�Ϊ�˶�����
public:
	int version_check;		//֧�ֵİ汾����Ϊ0
	int nbs_version;
	int nbs_vanilla_instrument_count;
	int nbs_length;
	int layer_count;
	string nbs_song_info[5];//����
	int nbs_song_stats[12]; //����
	nbs_head() { version_check = -1; }
	nbs_head(ifstream& infile);
};

//���캯��ֱ������ļ�����
nbs_head::nbs_head(ifstream& infile) {
	version_check = type_read(infile, Short);
	if (version_check != 0) return;//�������ϰ�nbs

	nbs_version = type_read(infile, Byte);
	nbs_vanilla_instrument_count = type_read(infile, Byte);
	nbs_length = type_read(infile, Short);
	layer_count = type_read(infile, Short);

	//�������޹ؽ�Ҫ���ļ�ͷ���ݣ����ÿ�
	for (int i = 0; i < 4; i++) type_read(infile, String, &nbs_song_info[i]);
	for (int i = 0; i < 9; i++) type_read(infile, nbs_head_seq[i + 9], &nbs_song_stats[i]);
	type_read(infile, String, &nbs_song_info[4]);
	for (int i = 9; i < 12; i++) type_read(infile, nbs_head_seq[i + 10], &nbs_song_stats[i]);
}



class nbs_note {
public:
	int tick, track, inst, key, vol, pan;	//����λ�ã����������ߣ�����������
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
	void refresh(ifstream& infile, int t, int tr) { //����ȡnbs�ļ�ʱʹ��
		tick = t;
		track = tr;
		inst = type_read(infile, Byte);
		key = type_read(infile, Byte);
		vol = type_read(infile, Byte);
		pan = type_read(infile, Byte);
		//pit = type_read(infile, Short);
	}
};



//����Ҫ���࣬��Ϊ����������Թ������
class nbs_track {
public:
	static int track_count;				//���ڴ���nbs�ļ���ʱ�Զ����
	string layer_name;
	int no;
	bool locked;
	int nbs_volume;
	int nbs_stereo;
	int inst;							//0-15��Ĭ��ÿ������ֻ��һ�����������õı���ϰ�ߣ����ɵ�һ����������������������Ϊ-1
	int track_length;					//���һ������tickλ�ã�������������Ŀ��
	vector<nbs_note*> notes;			//notes������ʼ��������track��������Ϣ��ʼ��Ϊ�գ�֮����copy_notes����

	nbs_track(nbs_track& t) {			//������̳������������ʹ��Ĭ�Ϲ��캯�������̳�������Ϣ
		layer_name = t.layer_name;
		no = t.no;
		locked = t.locked;
		nbs_volume = t.nbs_volume;
		nbs_stereo = t.nbs_stereo;
		inst = t.inst;
		track_length = -1;
	}
	nbs_track(string name, int num) {	//����������������������ʹ��Ĭ�Ϲ��캯��
		layer_name = name;
		no = num;
		locked = true;
		nbs_volume = 0;
		nbs_stereo = 0;
		inst = -1;
		track_length = -1;
	}
	nbs_track() {						//����nbs�ļ����ʼ��ʹ�ã��Զ����
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
	
	void refresh(ifstream& infile) {	//����ȡnbs�ļ�ʱʹ��
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

//track������Ϣ������˵��part3
void nbs_file::read_track_info(ifstream& infile) {
	for (int i = 0; i < head.layer_count; i++) {
		track[i].refresh(infile);
	}
}

//track������Ϣ������˵��part2
void nbs_file::construct_nbs_track(ifstream& infile) {
	//nbs�ļ���¼ÿ����������һ�������ġ����롱��������ʱ����룬�����������
	//��ȡʱ�����ϵ����ȶ���ÿ��tick�������죬�ٴ����Ҷ�������tick
	//�ú����Ӹ����ļ�ָ�루�Ѷ�λ����¼����Ĳ��֣�����ȡ�����¼���track����

	int h_pos = -1, v_pos = -1, h_jump, v_jump;
	bool tick_changed = false;			//��¼�Ƿ����У�ʱ�䣩��ת�������ж��������

	// h_jump, v_jump:������һ�����ģ�ˮƽʱ����ת���룬��ֱ������ת����
	// h_pos, v_pos: ��ǰʱ��λ�ã���ǰ����λ��

						
	do {								//����ˮƽ0����ֱ0�����н�����ˮƽ0����ֱ0��ˮƽ0���������������
		type_read(infile, Short, &h_jump);
		if (h_jump == 0)
			if (tick_changed) return;	//�����������
		h_pos += h_jump;
		type_read(infile, Short, &v_jump);
		if (v_jump == 0) {				//�н�������תtick
			v_pos = -1;
			tick_changed = true;
		}
		else {
			tick_changed = false;
			v_pos += v_jump;
			//�ҵ���һ������
			nbs_note* new_note = new(nbs_note);
			new_note->refresh(infile, h_pos, v_pos);
			track[v_pos].notes.push_back(new_note);
			track[v_pos].track_length= h_pos;
		}
	} while (true);
}

nbs_file::nbs_file(ifstream& infile):head(infile), track() {	//head���캯����ȡpart1
	nbs_track::track_count = 0;
	if (head.version_check != 0) return;						//��֧���ϰ�nbs��ʽ
	num_of_track = head.layer_count;
	construct_nbs_track(infile);								//��ȡpart2
	read_track_info(infile);									//��ȡpart3��part4���ԣ�
}

