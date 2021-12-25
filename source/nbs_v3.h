#pragma once
using namespace std;
#include <iostream>
#include <string>
#include <cmath>
#include <queue>
#include <algorithm>
#include "nbs_file_structure.h"

class nbs_file_v3;			//添加轨道分组group
class nbs_note_v3;			//添加延迟信息delay
class nbs_track_v3;			//整合分组group中各轨道的音符
class output_track;			//输出符合第三代编码机要求的音轨


class nbs_file_v3 : public nbs_file {
public:
	//专为第三代编码设计的部分，给音轨根据音色分组，相邻同音色的所有音轨自动归为一组
	vector<int> group[64];
	int num_of_group;

	nbs_file_v3() {};
	nbs_file_v3(ifstream& infile) :nbs_file(infile) {
		//构造函数自动实现分组
		num_of_group = 0;
		int last_instrument = -1, last_track = -1, track_in_group;
		for (int i = 0; i < num_of_track; i++) {
			if (!track[i].locked) {
				if (track[i].inst != last_instrument && track[i].inst != -1) {
					num_of_group++;
					track_in_group = 1;
					group[num_of_group - 1].push_back(i);
					last_track = i;
				}
				else if (track[i].inst == last_instrument && track[i].inst != -1) {
					track_in_group++;
					group[num_of_group - 1].push_back(i);
					last_track = i;
				}
				last_instrument = track[i].inst;
			}
		}
		num_of_track = last_track + 1;
	}
	~nbs_file_v3() {};
};



class nbs_note_v3 :public nbs_note {
public:
	int delay;							//添加延迟属性
	nbs_note_v3() :nbs_note() { delay = 0; }
	nbs_note_v3(nbs_note& n, int d) :nbs_note(n) { delay = d; }
	nbs_note_v3(nbs_note_v3& n, int d, int t, int tr) :nbs_note(n) { delay = d; tick = t; track = tr; }

	int report(string s) {
		if (s == "pitch") return key;
		else return delay;
	}
};

struct note_cmp {						//为生成nbs_track_v3准备的堆比较函数，时间靠前，音轨靠上的音符优先
	bool operator() (nbs_note* a, nbs_note* b) {
		if (a->tick == b->tick) return a->track > b->track;
		return a->tick > b->tick;
	}
};



class nbs_track_v3 :public nbs_track {
public:
	vector<nbs_note_v3*> notes;			//整合同一group的音轨中所有音符到notes
	int num_of_notes;

	nbs_track_v3() :nbs_track("Unnamed", -1) { num_of_notes = 0; }
	nbs_track_v3(nbs_track& t) :nbs_track(t) { copy_notes(t); }
	nbs_track_v3(nbs_track* t, vector<int> g) : nbs_track(t[g[0]]) { copy_notes(t, g); }
	nbs_track_v3(nbs_track_v3& t) :nbs_track(t) { copy_notes(t); }
	
	void copy_notes(nbs_track_v3& t) {
		notes = t.notes;
		num_of_notes = t.num_of_notes;
		track_length = t.track_length;
	}

	void copy_notes(nbs_track& t) {
		vector<int> g; g.push_back(0);
		copy_notes(&t, g);
	}

	void copy_notes(nbs_track* t, vector<int> g) {
		notes.clear();
		num_of_notes = 0;
		track_length = 0;

		int width = g.size();
		int pos[64]; memset(pos, 0, sizeof(pos));
		int pop_track;

		priority_queue<nbs_note*, vector<nbs_note*>, note_cmp> n;
		for (int i = 0; i < width; i++) {
			n.push(t[g[i]].notes[0]);
			num_of_notes += t[g[i]].notes.size();
		}
		while (notes.size() < num_of_notes) {
			if (notes.size() > 0 && n.top()->tick == track_length && n.top()->key == notes.back()->key)
				num_of_notes--;
			else 
				notes.push_back(new nbs_note_v3(*n.top(), n.top()->tick - track_length));
			track_length = n.top()->tick;
			pop_track = n.top()->track;
			n.pop();
			pos[pop_track]++;
			if (pos[pop_track] < t[pop_track].notes.size())
				n.push(t[pop_track].notes[pos[pop_track]]);
		}
	}

	void generate_outputs(vector<output_track*>& output, int init_delay);

	~nbs_track_v3() {
		if (notes.size() != 0) {
			for (int i = notes.size() - 1; i >= 0; i--)
				if (notes[i] != NULL)
					delete(notes[i]);
			notes.clear();
		}
	}
};



class output_track :public nbs_track_v3 {
public:
	int margin;

	output_track() :nbs_track_v3() { margin = 14; num_of_notes = 0; notes.clear(); }
	output_track(nbs_track_v3& t, int m) :nbs_track_v3(t) { margin = m; num_of_notes = 0; notes.clear(); }
	output_track(output_track& t) :nbs_track_v3(t), margin(t.margin) {}

	~output_track() {
		if (notes.size() != 0) {
			for (int i = notes.size() - 1; i >= 0; i--)
				if (notes[i] != NULL)
					delete(notes[i]);
			notes.clear();
		}
	};
};


bool track_cmp(output_track*& a, output_track*& b) {				//为生成output_track准备的比较函数，裕度更大，音符更多的输出音轨优先
	if (a->margin == b->margin) return a->num_of_notes < b->num_of_notes;
	return a->margin > b->margin;
};


//第三代编码核心算法
void nbs_track_v3::generate_outputs(vector<output_track*>& output, int init_delay) {

	//估算至少需要的输出音轨数量并建立音轨
	float note_density = float(num_of_notes) / float(track_length+15);
	int est = floor(note_density * 4) + 1;
	for (int i = 0; i < est; i++) {
		output_track* new_out = new output_track(*this, init_delay - 1);
		output.push_back(new_out);
	}
	
	//设置起始时刻
	int current_time = init_delay;
	int current_note = 0;
	int next_note_time = init_delay + notes[0]->delay;

	//开始分配
	while (true) {
		//刷新各轨缓存裕度
		for (int i = 0; i < output.size(); i++) {
			output[i]->margin++;
			if (current_time % 4 == 0) {
				if (output[i]->margin > 15)
					output[i]->margin -= 4;
			}
		}

		//若这一时刻有音符，读入音符
		while (current_time == next_note_time) {
			sort(output.begin(), output.end(), track_cmp);		//缓存裕度大的音轨优先写入

			//寻找合适的音轨t
			int t = 0, last_note_time; bool flag = true;
			for (int i = 0; i < output.size(); i++) {
				if (output[i]->margin >= 0) {					//缓存裕度必须非负
					if (output[i]->notes.empty()) { t = i; flag = false; break; } //若有新的空音轨，直接用
					last_note_time = output[i]->notes.back()->tick * 4 + output[i]->notes.back()->delay;
					if (current_time - last_note_time != 1) {	//距离上一个音不能是2gt
						t = i; flag = false; break;				//找到了合适的音轨
					}
				}
			}
			if (flag) {											//没有合适的音轨，新建一个
				t = output.size();
				output_track* new_out = new output_track(*this, 12 + current_time % 4);
				output.push_back(new_out);
			}

			//写入音符

			nbs_note_v3* new_note = new nbs_note_v3(*(this->notes[current_note++]), output[t]->margin, (current_time - output[t]->margin) / 4, t);
			new_note->key -= 33;
			if (output[t]->notes.size() < new_note->tick) {
				int gap = new_note->tick - output[t]->notes.size();
				for (int i = 0; i < gap; i++) {
					nbs_note_v3* new_empty_note = new nbs_note_v3(*new_note, 0, output[t]->notes.size(), t);
					new_empty_note->key = -1;
					output[t]->notes.push_back(new_empty_note);
				}
			}
			output[t]->notes.push_back(new_note);
			output[t]->num_of_notes++;
			output[t]->margin -= 4;

			//寻找下一个音符
			if (current_note < notes.size())
				next_note_time += this->notes[current_note]->delay;
			else break;
		}

		if (current_note < notes.size())
			current_time++;
		else break;
	}
}


