#include <cstdint>
#include <iostream>
#include <vector>
#include <hpx/hpx.hpp>
#include <hpx/hpx_init.hpp>
#include <hpx/util/format.hpp>
#include <hpx/include/partitioned_vector.hpp>
#include <hpx/runtime/find_localities.hpp>
#include <hpx/parallel/algorithms/for_loop.hpp>
#include <hpx/parallel/execution_policy.hpp>
#include <hpx/exception_list.hpp>

//	Must be always at the end, hpx-guideline	//

#include <boost/program_options.hpp>

	//	Standard values, if not specified	//

	double dx = 1.0;
	double dy = 1.0;
	double k = 1.0;		//	Heattransfer Coefficient //
	double dt = 0.05;	//	timestep //
	int iteration = 5; 
	int x = 5;
	int y = 5;
	bool print = 0;

template < typename T> void printVector(std::vector<std::vector<T> >& myvec){ 
	for(auto x=0; x<myvec.size();x++){ 
		for(auto y=0;y<myvec.at(x).size();y++){
			std::cout.precision(3); 
			std::cout << myvec.at(x).at(y) << std::setw(5) << " "; 
		} 
		std::cout << "\n"; 
	} 
}
	//	Checking, if accesing out of bounds element	//

int checkBounds(int i){

	if (i<0){
		return x-1;
	}else if (i==x)
	{
		return 0;
	}
	return i;
}

	// 2D Stencil implementation	//

double computeNewNode(std::vector<std::vector<double>>& arr, int i, int j){
	double x = dx*dx;
	double y = dy*dy;

	return ((arr[checkBounds(i-1)][j] + arr[checkBounds(i+1)][j] - 2*arr[i][j])/
		x) + ((arr[i][checkBounds(j-1)] + arr[i][checkBounds(j+1)] - 2*arr[i][j])/y);
}


//HPX_PLAIN_ACTION(computeNewNode, compute);

int hpx_main(boost::program_options::variables_map& vm){

    //	Initialize vectors	//

	std::vector<std::vector<double>> array(x,std::vector<double>(y,0.0));
	std::vector<std::vector<double>> array_new(x,std::vector<double>(y,0.0));

    //	Setting the heat in the left corner	//

    for (auto i = 1; i <= 1; ++i)
    {
    	for (auto j = 1; j <= 1; ++j)
    	{
    		array[i][j]=1.0;
    		array_new[i][j]=1.0;
    	}
    }

    hpx::parallel::execution::parallel_task_policy p;

    hpx::exception_list e;
    hpx::future<void> f;
    
	//	Setting clock for measurment	//
    std::uint64_t past = hpx::util::high_resolution_clock::now();

    

	for (auto i = 1; i <= iteration; ++i)
	{
		//	Swapping vectors after each round of iteration 	//

	    std::swap(array,array_new);

	   // for (auto i = 0; i <= x-1; ++i)
	    //{
	    	f = hpx::parallel::v2::for_loop(
	    		p, 0, x,
	    		[&array, &array_new] (int i){
			//array_new[i][j] = array[i][j] + computeNewNode(array,i,j)* dt * k;
				for (int j = 0; j < y; ++j)
				{
					array_new[i][j] = array[i][j] + computeNewNode(array,i,j)* dt * k;	
				}
	    	});
	    	f.get();
	    //}

	}



	//	Calculating computing time 	//
	std::uint64_t elapsed = hpx::util::high_resolution_clock::now() - past;
	
	if(print != 0){
	printVector(array);
	std::cout << "\n";
	}


	//	14, round, diveded by 1e9, hpx uses hust a wrapper for boost::format //
	
	hpx::util::format_to(std::cout, "%.14g" ,elapsed/1e9) << "\n";

    return hpx::finalize();
}

int main(int argc, char* argv[])
{
    
    namespace po = boost::program_options;
    po::options_description desc("Allowed options");
    desc.add_options()
                ("help,h", "Print this Message")
                ("iteration,i", po::value<int>(&iteration),
                 "Set Iterations, standart 5")
                ("timestep,d", po::value<double>(&dt),
                 "Set timestep, standart 0.05")
                ("heattrans,h",po::value<double>(&k),
                 "Set heattransfer coefficent, standart 1.0")
                ("sizex,x", po::value<int>(&x), 
                 "Set number of columns ")
                ("sizey,y", po::value<int>(&y), 
                 "Set number of rows")
                ("output,o", po::value<bool>(&print), 
                 "Display the Data")
                ;

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if(vm.count("help")) {
        std::cout << desc << "\n";
        return 1;
    }


	return hpx::init(desc,argc, argv);
}
