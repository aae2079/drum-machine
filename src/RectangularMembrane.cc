#include "RectangularMembrane.h"


RectangularMembrane::RectangularMembrane(int nx, int ny, float damp, float c, float time_step, float sim_time)
    : nx_(nx), ny_(ny), damp_(damp), c_(c), time_step_(time_step), sim_time_(sim_time) {
    // Constructor implementation
}

RectangularMembrane::RectangularMembrane() 


    // comstructor  builds the grid and sets initla conditions 
    : nx_(GRID_X), ny_(GRID_Y), damp_(10.0), c_(1.0), time_step_(1.0/SAMPLE_RATE), sim_time_(2.0) {
    
    // Instantiate grids
    curr_ = std::vector<float>(nx_ * ny_, 0.0f);
    prev_ = std::vector<float>(nx_ * ny_, 0.0f);
    next_ = std::vector<float>(nx_ * ny_, 0.0f);

    /*
            nx
    |0 0 0 0 0 0 0 0 0 0|
    |0 . . . . . . . . 0|
    |0 . . . . . . . . 0| ny
    |0 . . . . . . . . 0|
    |0 0 0 0 0 0 0 0 0 0|
    
    */
    for (int ii = 0; ii < nx_; ii++){
        curr_[ii] = 0.0f;
        curr_[ii + (ny_ - 1) * nx_] = 0.0f;
        prev_[ii] = 0.0f;
        prev_[ii + (ny_ - 1) * nx_] = 0.0f;
    }

    for (int jj = 0; jj < ny_; jj ++){
        curr_[jj * nx_] = 0.0f;
        curr_[(jj + 1) * nx_ - 1] = 0.0f;
        prev_[jj * nx_] = 0.0f;
        prev_[(jj + 1) * nx_ - 1] = 0.0f;
    }

    setInitialCondition();


}

RectangularMembrane::~RectangularMembrane(){
    // Destructor implementation (if needed)
    prev_.clear();
    curr_.clear();
    next_.clear();
    histBuf_.clear();
    audioBuf_.clear();
}

void RectangularMembrane::setInitialCondition(){
    /* 
        This method is very bare but the intention here is that this method will
        set the initial displacement based off how the user interacts with the 
        membrane on the GUI. For now we will just hard code a gaussian strike at the center
        baby steps ....
    */

    // We will start with a simple gaussian strike/pluck
    int center_x = nx_ / 2;
    int center_y = ny_ / 2;
    float amp = 0.1;

    #pragma omp parallel for
    for (int ii = 1; ii < nx_-1; ii ++){
        for (int jj = 1; jj < ny_-1; jj ++){
            //ampIn*exp(-alpha*(((i-1)-x_mid)^2+((j-1)-y_mid)^2));
            curr_[ii + jj * nx_] = amp * exp(-0.01*(pow(((ii-1)-center_x),2) + pow(((jj-1)-center_y),2)));
            prev_[ii + jj * nx_] = curr_[ii + jj * nx_];   
        }
    }
    

}

void RectangularMembrane::Simulate(){
    /* Main simulation loop */
    audioBuf_.resize((int)BUFFER_SIZE, 0.0f);
    histBuf_.resize((int)OVERLAP, 0.0f);
    std::vector<float> curBuf(BUFFER_SIZE, 0.0f);

    /*
    // Discretized equation:
    u^(n+1)_(i,j) = 
    [1 - ηΔt/2] { ρ[u^n_(i+1,j) + u^n_(i-1,j) + 
                    u^n_(i,j-1) + u^n_(i,j+1) - 4u^n_(i,j)] + 
                    2u^n_(i,j) - [1 - ηΔt/2]u^(n-1)_(i,j) }
    
    */

    //this needs to be done in chunks for real time simulating
    for (int tt = 0; tt < (int)BUFFER_SIZE; tt++){

        std::fill(next_.begin(), next_.end(), 0.0f); // Clear next grid
        
        #pragma omp parallel for
        for (int ix = 1; ix < nx_ - 1; ix++){
            for (int iy = 1; iy < ny_ - 1; iy++){
                next_[ix + iy * nx_] = (1.0 / (1 + (damp_ * time_step_ / 2.0))) * (CFL * (curr_[ix+1 + iy * nx_] + 
                            curr_[ix-1 + iy * nx_] + 
                            curr_[ix + (iy+1) * nx_] + 
                            curr_[ix + (iy-1) * nx_] - 
                            4.0 * curr_[ix + iy * nx_]) +
                   2.0 * curr_[ix + iy * nx_] - 
                   (1.0 - (damp_ * time_step_ / 2.0)) * prev_[ix + iy * nx_]);
            }
        }
        // Update grids for next time step
        std::swap(prev_, curr_);
        std::swap(curr_, next_);        

        // Store displacments at a specific point for audio output
        curBuf[tt] = curr_[nx_ / 2 + (ny_ / 2) * nx_];
    }

    if (firstTime){
        audioBuf_ = curBuf;
        std::copy(curBuf.end() - (int)OVERLAP, curBuf.end(), histBuf_.begin()); // get the last OVERLAP samples
        firstTime = false;
    } else {
        // Append new chunk to audio buffer
        audioBuf_.clear();
        audioBuf_.insert(audioBuf_.end(), histBuf_.begin(), histBuf_.end());
        audioBuf_.insert(audioBuf_.end(), curBuf.begin(), curBuf.end() - (int)OVERLAP);

        // Reset histBuf_ to last OVERLAP samples of curBuf for next chunk
        histBuf_.clear();
        std::copy(curBuf.end() - (int)OVERLAP, curBuf.end(), histBuf_.begin());
    }
}