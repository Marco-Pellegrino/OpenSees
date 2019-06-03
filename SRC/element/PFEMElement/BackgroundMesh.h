/* ****************************************************************** **
**    OpenSees - Open System for Earthquake Engineering Simulation    **
**          Pacific Earthquake Engineering Research Center            **
**                                                                    **
**                                                                    **
** (C) Copyright 1999, The Regents of the University of California    **
** All Rights Reserved.                                               **
**                                                                    **
** Commercial use of this program without express permission of the   **
** University of California, Berkeley, is strictly prohibited.  See   **
** file 'COPYRIGHT'  in main directory for information on usage and   **
** redistribution,  and for a DISCLAIMER OF ALL WARRANTIES.           **
**                                                                    **
** Developed by:                                                      **
**   Frank McKenna (fmckenna@ce.berkeley.edu)                         **
**   Gregory L. Fenves (fenves@ce.berkeley.edu)                       **
**   Filip C. Filippou (filippou@ce.berkeley.edu)                     **
**                                                                    **
** ****************************************************************** */

// $Revision$
// $Date$
// $URL$

// Written: Minjie Zhu
//
// Description: This class defines the BackgroundMesh
//

#ifndef BackgroundMesh_h
#define BackgroundMesh_h

#include "BackgroundDef.h"
#include <vector>
#include <set>
#include <ID.h>
#include <Node.h>
#include <Recorder.h>
#include <fstream>

class BackgroundMesh
{
private:

    static int FLUID, STRUCTURE, FIXED;

    // FLUID - a grid fluid node
    // STRUCTURE - a structural node
    // FIXED - a fixed grid fluid node
    struct BNode {

        VInt tags;
        VVDouble crdsn;
        VVDouble vn;
        VVDouble dvn;
        VDouble pn;
        VDouble dpn;
        VInt type;

        BNode():tags(),crdsn(),vn(),dvn(),pn(),dpn(),type(){}
        void addNode(int tag, const VDouble& crds, const VDouble& v,
                     const VDouble& dv, double p, double dp, int tp) {
            tags.push_back(tag);
            crdsn.push_back(crds);
            vn.push_back(v);
            dvn.push_back(dv);
            pn.push_back(p);
            dpn.push_back(dp);
            type.push_back(tp);
        }
        void clear() {
            tags.clear();
            crdsn.clear();
            vn.clear();
            dvn.clear();
            pn.clear();
            dpn.clear();
            type.clear();
        }
        void addNode(int tp) {
            addNode(0,VDouble(),VDouble(),VDouble(),0,0,tp);
        }
        int size() const {return (int)tags.size();}
        void getNode(int i, int& tag, VDouble& crds, VDouble& v,
                     VDouble& dv, double& p, double& dp, int& tp) {
            if (i<0 || i>=(int)tags.size()) return;
            tag = tags[i];
            crds = crdsn[i];
            v = vn[i];
            dv = dvn[i];
            p = pn[i];
            dp = dpn[i];
            tp = type[i];
        }
        void setNode(int i, int tag, const VDouble& crds, const VDouble& v,
                     const VDouble& dv, double p, double dp, int tp) {
            if (i<0 || i>=(int)tags.size()) return;
            tags[i] = tag;
            crdsn[i] = crds;
            vn[i] = v;
            dvn[i] = dv;
            pn[i] = p;
            dpn[i] = dp;
            type[i] = tp;
        }

    };

    // FLUID - a grid fluid cell
    // STRUCTURE - a structural cell, which should have no particles
    struct BCell {
        VParticle pts;
        int type;
        std::vector<BNode*> bnodes;
        std::vector<VInt> bindex;

        BCell():pts(),type(FLUID),bnodes(),bindex(){}
        void add(Particle* pt) {pts.push_back(pt);}
    };

public:
    BackgroundMesh();
    virtual ~BackgroundMesh();

    // background info
    void setTol(double t) {this->tol = t;}
    double getTol() const {return this->tol;}
    void setMeshTol(double t) {meshtol = t;}
    double getMeshTol() const {return meshtol;}
    void setRange(const VDouble& l, const VDouble& u);
    void setBasicSize(double size) {bsize = size;}
    void addRecorder(Recorder* recorder);
    int record(bool init=false);
    void setLocs(const VDouble& l) {this->locs = l;}
    int setFile(const char* name);
    void setNumSub(int num) {numsub = num;}
    void addStructuralNodes(VInt& snodes);
    void setKernel(const char* k, bool = false);

    // remesh all
    int remesh(bool init=false);

    // get grids
    void getIndex(const VDouble& crds, double incr, VInt& index) const;
    void lowerIndex(const VDouble& crds, VInt& index) const;
    void upperIndex(const VDouble& crds, VInt& index) const;
    void nearIndex(const VDouble& crds, VInt& index) const;
    void getCrds(const VInt& index, VDouble& crds) const;
    void getCorners(const VInt& index, int num, VVInt& indices) const;

    // particles
    int addParticles();
    void gatherParticles(const VInt& minindex, const VInt& maxindex,
                         VParticle& pts, bool checkfsi=false);
    int moveParticles();
    int convectParticle(Particle* pt, VInt index, int nums);
    int moveFixedParticles();

    // create grid nodes and elements
    int addWall(const VDouble& low, const VDouble& up);
    int addStructure();
    int gridNodes();
    int gridFluid();
    int gridFSI(ID& freenodes);
    int gridEles();

    // particle kernel
    static double QuinticKernel(double q, double h, int ndm);
    static int preNForTri(double x1, double y1, double x2, double y2,
                          double x3, double y3, VDouble& coeff);
    void getNForTri(const VDouble& coeff, double x, double y,
                    VDouble& N);
    static void getNForRect(double x0, double y0, double hx, double hy,
                            double x, double y, VDouble& N);

    static int preNForTet(const VDouble& crds1, const VDouble& crds2,
                          const VDouble& crds3, const VDouble& crds4,
                          VVDouble& coeff);
    void getNForTet(const VVDouble& coeff, const VDouble& crds,
                    VDouble& N);
    static void getNForRect(double x0, double y0, double z0,
                            double hx, double hy, double hz,
                            double x, double y, double z,
                            VDouble& N);


    // clear all
    int clearBackground();
    static void clearGridEles();
    void clearGrid();

    // interpolate in a cell
    static int interpolate(Particle* pt, const VVInt& index,
                           const VVDouble& vels, const VVDouble& dvns,
                           const VDouble& pns, const VDouble& dpns,
                           const VVDouble& crds,
                           const VInt& fixed, VDouble& pvel);
    static int solveLine(const VDouble& p1, const VDouble& dir,
                         int dim, double crd, double& k);
    static bool inEle(const VDouble& N);

    // find free surface
    int findFreeSurface(const ID& freenodes);
    void setFreeSurface() {freesurface = true;}

private:

    VInt lower, upper;
    VVDouble wlower, wupper;
    std::map<VInt,BCell> bcells;
    std::map<VInt,BNode> bnodes;
    double tol, meshtol;
    double bsize;
    int numave, numsub;
    std::vector<Recorder*> recorders;
    VDouble locs;
    double currentTime;
    std::ofstream theFile;
    std::set<int> structuralNodes;
    bool freesurface;
    VInt sptags;
    int kernel, pkernel; // 1 - QuinticKernel, 2 - CloestKernel
    int tsTag, loadPatternTag;
};


#endif
