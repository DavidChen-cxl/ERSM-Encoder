#pragma once
using namespace std;
#include <iostream>
#include <vector>
#include <cmath>
#include "nbs_file_structure.h"
#include "shulker_output.h"

class nbs_note_v2:public nbs_note {
public:
	int repeat;
	nbs_note_v2() :nbs_note() { repeat = 0; }
	nbs_note_v2(nbs_note_v2& n) :nbs_note(n) { repeat = n.repeat; }
	nbs_note_v2(nbs_note& n, int r) :nbs_note(n) {
		tick /= 4;
		repeat = r;
	}
	int report(string s) {
		if (s == "pitch") return key;
		else return repeat;
	}
};

class nbs_track_v2 :public nbs_track {
public:
	vector<nbs_note_v2*> notes;
	vector<int> error;

	nbs_track_v2() :nbs_track("Unnamed", -1) {}
	nbs_track_v2(nbs_track& t) : nbs_track(t) {
		for (int i = 0; i < t.notes.size(); i++) {
			if (t.notes[i]->key <= 57 && t.notes[i]->key >= 33)
				if (push_note(*(t.notes[i]))) continue;
			if (error.size()==0||error.back() != t.notes[i]->tick / 4)
				error.push_back(t.notes[i]->tick / 4);
		}
	}

	bool push_note(nbs_note& n) {
		if (notes.size() < n.tick / 4) {
			int gap = n.tick / 4 - notes.size();
			for (int i = 0; i < gap; i++) {
				nbs_note_v2* new_note = new nbs_note_v2(n, 0);
				new_note->key = -1;
				notes.push_back(new_note);
			}
		}
		if (notes.size() == n.tick / 4) {
			nbs_note_v2* new_note = new nbs_note_v2(n, pow(2, 3 - n.tick + 4 * notes.size()));
			new_note->key -= 33;
			notes.push_back(new_note);
			return true;
		}
		else if (notes.back()->key + 33 == n.key) {
			notes.back()->repeat += pow(2, 3 - n.tick + 4 * notes.back()->tick);
			return true;
		}
		else {
			return false;
		}
	}

	int err_report(ofstream& f) {
		if (error.size()) {
			f << "#For Track " << layer_name << " found errors at bar ";
			for (int i = 0; i < error.size(); i++)
				f << error[i] + 1 << ", ";
			f << "\n\n";
			return error.size();
		}
		else return 0;
	}
};
