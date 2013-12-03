#ifndef SCANORDER_H_
#define SCANORDER_H_


class ScanOrderIntra4x4 {
private:
	static int intra_4x4_scan_order[16][2];
public:
	static int getX(int idx, int x);
	static int getY(int idx, int y);
};


#endif /* SCANORDER_H_ */
