/*
	Tele2d: This software implements the 2d tele-registration method in the paper,

	ARTICLE{Telereg2013
	title = {¡°Mind the Gap¡±: Tele-Registration for Structure-Driven Image Completion},
	author = {H. Huang and K.Yin and M. Gong and D. Lischinski and D. Cohen-Or and U. Ascher and B. Chen},
	journal = {ACM Transactions on Graphics (Proceedings of SIGGRAPH ASIA 2013)},
	volume = {32},
	issue = {6},
	pages = {174:1--174:10},
	year = {2013},
	}

	Copyright (C) <2014>  <Kangxue Yin - yinkangxue@gmail.com>

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include <stdlib.h>
#include <math.h>
#include "..\thirdpart\glut\glut.h"
#include <vector>
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include "..\thirdpart\nlopt-2.3\nlopt.hpp"




#include "..\telereg-lib\tele2d\tele2d.h"

tele2d *teleRegister ;
#ifdef _DEBUG 
#pragma comment(lib, "telereg-libd.lib") 
#else
#pragma comment(lib, "telereg-lib.lib") 
#endif


State appstate = BEFORE_LOAD_CURVES ;			// state of  application

extern int display_original ;

void keyboard(unsigned char key, int x, int y ){

	std::cout<<key<<std::endl;
	std::cout<<(int)key<<std::endl;



	int a =glutGetModifiers();
	if( (key == 'r' || key == 'R') && appstate == AFTER_LOAD_CURVES  ){ 
		teleRegister->runRegister() ;
		teleRegister->setResultField() ;
		appstate = SHOW_RESULT ;
	}


	if(( key == 'q'||key == 'Q'||key==9 )&& appstate == SHOW_RESULT ){					// press q/0 to switch between original fields and new fields

		if( display_original == 1 ){
			display_original = 0 ;
			teleRegister->setResultField() ;
			
		}
		else{
			display_original = 1;
			teleRegister->setInputField() ;
		}
	}



	if( key == 19 && ( glutGetModifiers() & GLUT_ACTIVE_CTRL )  ){ // press ctrl+s to save image and curves
		teleRegister->outputResCurves("rescurves.txt") ;
	}

	glutPostRedisplay();
	
}

void myreshape( GLsizei w, GLsizei h){
	glutReshapeWindow(800,800) ;
}

void display(void) ;

#define safe_load_curve(X) if(!(X) ){ \
	std::cout << "cannot load curve file: "<< fname <<std::endl;  \
	return false ;\
} ;

bool load_Curves( std::string fname, 	CURVES &curves , std::vector<std::vector<int>> &curves_group, std::vector<int2> &endpoints  ){

			curves.clear() ;
			curves_group.clear() ;
			endpoints.clear() ;

			std::ifstream ifs( fname ) ;

			safe_load_curve(ifs) ;

			// load curves
			int curve_num ;
			safe_load_curve( ifs >> curve_num ) ; 

			for( int i=0; i<curve_num; ++i){
				int len ;
				curves.resize(curves.size() + 1 ) ;
				safe_load_curve( ifs >> len ) ; 
				for( int i=0; i<len; ++i ){
					double x, y ;
					safe_load_curve( ifs>>x>>y );
					curves.back().push_back( double2(x, y) ) ;
				}

			}

			//  interpolate short curves
			for(int i=0; i<curves.size(); ++i ){
				std::vector<double2> cur = curves[i];
				while( cur.size() < 10 ){
					std::vector<double2> cur1 ;
					cur1.push_back( cur[0] ) ;

					for( int j=1; j<cur.size(); ++j ){
						cur1.push_back( (cur[j-1] + cur[j]) * 0.5 ) ;
						cur1.push_back( cur[j] ) ;
					}
					cur = cur1 ;
				}
				curves[i] = cur ;
			}


			// load groups
			int gnum ;
			safe_load_curve( ifs >> gnum ) ; 
			for( int i=0; i<gnum; ++i){
				int len ;
				curves_group.resize(curves_group.size() + 1 ) ;
				safe_load_curve( ifs >> len ) ; 
				for( int i=0; i<len; ++i ){
					int cid ;
					safe_load_curve(ifs>>cid)  ;
					curves_group.back().push_back( cid ) ;
				}

			}


			// load endpoints
			endpoints.resize(curves.size());
			int tmp ;
			while( ifs>>tmp ){
				//howtoconnect.push_back(tmp);

				if( tmp%2 )
					endpoints[tmp/2].y = 1 ;
				else
					endpoints[tmp/2].x = 1 ;
			}

			return true;

}

int main(int argc, char **argv) {




	teleRegister = new tele2d( 50, 0.02,1 ) ;

	CURVES curves ;
	std::vector<std::vector<int>> group ;
	std::vector<int2> endps ;


	if( argc < 2 )	
		load_Curves( "curves.txt", curves, group, endps );
	else 
		load_Curves(argv[1], curves, group, endps );


	teleRegister->init( curves, group, endps  ) ;


	//// Uncomment the 3 lines below, you can directly run the registration and save the result.
	//teleRegister->runRegister() ;
	//teleRegister->outputResCurves( "rescurves.txt") ;
	//return 0;



	teleRegister->setInputField() ; // only for visualization
	appstate = AFTER_LOAD_CURVES ;


	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_SINGLE | GLUT_RGBA);
	glEnable( GL_POINT_SMOOTH  ) ;
	glutInitWindowPosition(800,100);
	glutInitWindowSize(800,800);
	glutCreateWindow("Tele-Registration");
	glutDisplayFunc(display);
	//glutMouseFunc( mouse ) ;
	//glutMotionFunc( motionFunc );
	glutKeyboardFunc( keyboard );
	//glutSpecialFunc( specialKeyboard);
	glutReshapeFunc(myreshape) ;


	glutMainLoop();

	
	return 0;
}

