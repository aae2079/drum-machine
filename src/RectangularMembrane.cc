#include "RectangularMembrane.h"


RectangularMembrane::RectangularMembrane(int nx, int ny, float damp, float c, float time_step, float sim_time)
    : nx_(nx), ny_(ny), damp_(damp), c_(c), time_step_(time_step), sim_time_(sim_time) {
    // Constructor implementation
}

RectangularMembrane::RectangularMembrane() 
    : nx_(128), ny_(128), damp_(10.0), c_(1.0), time_step_(1.0/SAMPLE_RATE), sim_time_(2.0) {
    
    // Instantiate grids
    curr_ = std::vector<std::vector<float>>(nx_, std::vector<float>(ny_, 0.0f));
    prev_ = std::vector<std::vector<float>>(nx_, std::vector<float>(ny_, 0.0f));
    next_ = std::vector<std::vector<float>>(nx_, std::vector<float>(ny_, 0.0f));

    // Initialize time vector
    num_samples_ = SAMPLE_RATE * sim_time_; 

    // Set boundary conditions
    for (int ii = 0; ii < nx_; ii ++){
        curr_[ii][0] = 0.0f;
        curr_[ii][ny_ -1] = 0.0f;
        prev_[ii][0] = 0.0f;
        prev_[ii][ny_ -1] = 0.0f;
    }

    for (int jj = 0; jj < ny_; jj ++){
        curr_[0][jj] = 0.0f;
        curr_[nx_ -1][jj] = 0.0f;
        prev_[0][jj] = 0.0f;
        prev_[nx_ -1][jj] = 0.0f;
    }

}

RectangularMembrane::~RectangularMembrane(){
    // Destructor implementation (if needed)
    prev_.clear();
    curr_.clear();
    next_.clear();
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

    for (int ii = 1; ii < nx_-1; ii ++){
        for (int jj = 1; jj < ny_-1; jj ++){
            //ampIn*exp(-alpha*(((i-1)-x_mid)^2+((j-1)-y_mid)^2));
            curr_[ii][jj] = amp * exp(-0.01*(pow(((ii-1)-center_x),2) + pow(((jj-1)-center_y),2)));
            prev_[ii][jj] = curr_[ii][jj];   
        }
    }

}

void RectangularMembrane::Simulate(std::vector<float>& output_buffer){
    /* Main simulation loop */
    output_buffer.resize(num_samples_, 0.0f);

    /*
    // Discretized equation:
    u^(n+1)_(i,j) = 
    [1 - ηΔt/2] { ρ[u^n_(i+1,j) + u^n_(i-1,j) + 
                    u^n_(i,j-1) + u^n_(i,j+1) - 4u^n_(i,j)] + 
                    2u^n_(i,j) - [1 - ηΔt/2]u^(n-1)_(i,j) }
    
    */
    for (int tt = 0; tt < num_samples_; tt++){
        
        for (int ii = 0; ii < nx_ ; ii++)
            std::fill(next_[ii].begin(), next_[ii].end(), 0.0f);
        
        for (int ix = 1; ix < nx_ - 1; ix++){
            for (int iy = 1; iy < ny_ - 1; iy++){
                next_[ix][iy] = (1.0 / (1 + (damp_ * time_step_ / 2.0))) * (CFL * (curr_[ix+1][iy] + 
                            curr_[ix-1][iy] + 
                            curr_[ix][iy+1] + 
                            curr_[ix][iy-1] - 
                            4.0 * curr_[ix][iy]) +
                   2.0 * curr_[ix][iy] - 
                   (1.0 - (damp_ * time_step_ / 2.0)) * prev_[ix][iy]);
            }
        }
        // Update grids for next time step
        std::swap(prev_, curr_);
        std::swap(curr_, next_);        

        // Store displacments at a specific point for audio output
        output_buffer[tt] = curr_[nx_ / 2][ny_ / 2];
    }
}