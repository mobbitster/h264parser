#ifndef __PREDICTION_H
#define __PREDICTION_H

namespace h264 {

struct mv {
	int x,y;
	int available;  // i.e. inside the image
	int valid;      // i.e. usable for prediction
};

class Prediction {
public:
	enum {
		MV_NA	= 0x80808080,
	};

public:
// per-macroblock information     (16x16)
	int mb_width, mb_height, mb_pitch;
	int *mb_mode;
 // per-chroma block information    (8x8)
	int cb_width, cb_height, cb_pitch;
	int *total_coeff_c[2];
 // per-transform block information (4x4)
	int tb_width, tb_height, tb_pitch;
	int *total_coeff_l;
	int *intra_4x4_pm;
	int *mvx,*mvy;

private:
	bool allocated;

public:
	Prediction() : allocated(false) {};

	~Prediction();

	bool isAllocated() {
		return this->allocated;
	}

	void allocate(int width, int height);
	void clear() const;

	int getMacroblockMode(int x, int y) const {
		return this->mb_mode[(y) * this->mb_pitch + (x)];
	}

	void setMacroblockMode(int x, int y, int mode) const {
		this->mb_mode[(y) * this->mb_pitch + (x)];
	}

	int getIntra4x4PredMode(int x, int y) const {
		if (x < 0 || y < 0)
			return -1;
		return this->intra_4x4_pm[(y >> 2) * this->tb_pitch + (x >> 2)];
	}

	void setIntra4x4PredMode(int x, int y, int val) const {
		if (x < 0 || y < 0)
			return;
		this->intra_4x4_pm[(y >> 2) * this->tb_pitch + (x >> 2)] = val;
	}

	int calculateIntra4x4PredMode(int x, int y) const;

	int getTotalCoeffL(int x, int y) const {
		return this->total_coeff_l[(y) * this->tb_pitch + (x)];
	}

	void setTotalCoeffL(int x, int y, int val) const {
		this->total_coeff_l[(y) * this->tb_pitch + (x)] = val;
	}

	int getTotalCoeffC(int x, int y, int iCbCr) const {
		return this->total_coeff_c[iCbCr][(y) * this->cb_pitch + (x)];
	}

	void setTotalCoeffC(int x, int y, int iCbCr, int val) const {
		this->total_coeff_c[iCbCr][(y) * this->cb_pitch + (x)] = val;
	}

	int getLumaNN(int x, int y) const;
	int getChromaNN(int x, int y, int iCbCr) const;
	int getLumaNC(int x, int y) const;
	int getChromaNC(int x, int y, int iCbCr) const;

	void setMVx(int x, int y, int val) {
		this->mvx[(y) * this->tb_pitch + (x)] = val;
	}

	void setMVy(int x, int y, int val) {
		this->mvy[(y) * this->tb_pitch + (x)] = val;
	}

	int getMVx(int x, int y) {
		return this->mvx[(y) * this->tb_pitch + (x)];
	}

	int getMVy(int x, int y) {
		return this->mvy[(y) * this->tb_pitch + (x)];
	}

	struct mv getMV(int x, int y);

	struct mv predictMV(int org_x, int org_y, int width, int height);
	struct mv predictMVSkip(int org_x, int org_y);

	void fillMV(int org_x, int org_y, int width, int height, int mvx, int mvy);
	void deriveMVSkip(int org_x, int org_y);
	void deriveMV(int org_x, int org_y, int width, int height, int mvdx, int mvdy);

};

}

#endif
