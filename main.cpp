#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/videoio.hpp>
#include <iostream>
#include <string>
#include <vector>

using namespace cv;
using namespace std;

const int CRITERE_HOMOGENEITE = 50;
const int CRITERE_SIMILARITE = 15;

//Région défini par un point au coordonnée (x,y), une largeur, une hauteur et une couleur
struct Region {
    int x;
    int y;
    int width;
    int height;
    int color;
};

//Fonction de comparaison de région
bool CompRegion(Region r1, Region r2) {
    if((r1.x == r2.x) && (r1.y == r2.y) ) {
         return true;
    } else {
        return false;
    }
}

//Fonction qui retourne la couleur d'une région (couleur moyenne de la région)
int colorRegion(Region r1, Mat src) {
    int totalcolor = 0;
    for(int i = r1.x ; i < r1.x + r1.width; i++)
    {
        for(int j = r1.y; j < r1.y + r1.height; j++)
        {
            int pixelcolor = src.at<Vec3b>(j,i)[0];
            totalcolor = totalcolor + pixelcolor;
        }
    }

    int totalpixel = r1.width*r1.height;
    return totalcolor / totalpixel;
}

//Fonction qui retourne la couleur du pixel ave le plus petit niveau de gris (minimum) de la région
int minColorRegion(Region r1, Mat src) {
    int mincolor = src.at<Vec3b>(r1.y,r1.x)[0];
    for(int i=r1.x ; i<r1.x + r1.width;i++) {
        for(int j=r1.y; j<r1.y+r1.height;j++) {
            int coloractu = src.at<Vec3b>(j,i)[0];
            if(coloractu < mincolor) {
                mincolor = src.at<Vec3b>(j,i)[0];
            }
        }
    }

    return mincolor;
}

//Fonction qui retourne la couleur du pixel avec le plus haut niveau de gris (maximum) de la région
int maxColorRegion(Region r1, Mat src) {
    int maxcolor = src.at<Vec3b>(r1.y,r1.x)[0];
    for(int i=r1.x; i < r1.x+r1.width;i++) {
        for(int j=r1.y; j < r1.y+r1.height;j++) {
            int coloractu = src.at<Vec3b>(j,i)[0];
            
            if(coloractu > maxcolor) {
                maxcolor = src.at<Vec3b>(j,i)[0];
            }
        }
    }
    return maxcolor;
}

//Fonction qui retourne vrai si deux région sont similaire
//critère de similarité : différence de couleur entre les 2 régions
bool RegionSimilaire(Region r1, Region r2) {
    int diffcolor = abs(r2.color - r1.color);

    if(diffcolor < CRITERE_SIMILARITE) {
        return true;
    } else {
        return false;
    }
}

//Fonction qui retourne vrai si les régions en paramètres sont adjacentes
bool RegionAdjacent(Region region1, Region region2) {
    int x = max(0, min(region1.x + region1.width, region2.x + region2.width) - max(region1.x, region2.x)); //calcul le chevauchement en x (horizontal)
    int y = max(0, min(region1.y + region1.height, region2.y + region2.height) - max(region1.y, region2.y)); //calcul le chevauchement en y (vertical)
    bool xcolle =false;
    bool ycolle =false;
    if(region1.x + region1.width == region2.x) {
        xcolle = true;
    } 
    if(region1.y + region1.height == region2.y) {
        ycolle = true;
    }
    //les régions sont adjacentes en x si x est supérieur à 0 et que y est égal à 0 et en y si x est égal à 0 et que y est supérieur à 0
    return (x > 0 && y == 0 && ycolle == true) || (x == 0 && y > 0 && xcolle == true);
}

//Fonction qui "split"/divise les régions en fonction d'un critère d'homogénéité
//critère d'homogénéité : différence entre le niveau de gris maximum et minimum au sein de la région 
bool splitRegion(Region r, vector<Region> &VectorRegion, Mat &src, bool changement) {
    Region r1;
    Region r2;
    Region r3;
    Region r4;

    r1.x = r.x;
    r1.y= r.y;
    r1.width =r.width/2;
    r1.height=r.height/2;
    r1.color=colorRegion(r1,src);

    r2.x = r.x + r.width/2;
    r2.y= r.y;
    r2.width =r.width/2;
    r2.height=r.height/2;
    r2.color=colorRegion(r2,src);

    r3.x = r.x;
    r3.y= r.y + r.height/2;
    r3.width = r.width/2;
    r3.height= r.height/2;
    r3.color=colorRegion(r3,src);

    r4.x = r.x + r.width/2;
    r4.y= r.y + r.height/2;
    r4.width =r.width/2;
    r4.height=r.height/2;
    r4.color=colorRegion(r4,src);

    int position;
    int sizevector = VectorRegion.size();
    for(int i=0;i<sizevector;i++){
        if(CompRegion(r,VectorRegion[i])) {
        position = i;
        }
    }
    VectorRegion.erase(VectorRegion.begin()+ position);
    VectorRegion.insert(VectorRegion.begin()+ position, r1);
    VectorRegion.insert(VectorRegion.begin()+ position + 1, r2);
    VectorRegion.insert(VectorRegion.begin()+ position + 2, r3);
    VectorRegion.insert(VectorRegion.begin()+ position + 3, r4);

    if(changement) {
        if( ( (maxColorRegion(r1,src) - minColorRegion(r1,src) ) > CRITERE_HOMOGENEITE) && (r1.width> 5) && (r1.height>5) ) {
            splitRegion(r1,VectorRegion,src,true);
        }
        if(( (maxColorRegion(r2,src) - minColorRegion(r2,src) ) > CRITERE_HOMOGENEITE) && (r2.width> 5) && (r2.height>5)) {
            splitRegion(r2,VectorRegion,src,true);
        }
        if(( (maxColorRegion(r3,src) - minColorRegion(r3,src) ) > CRITERE_HOMOGENEITE) && (r3.width> 5) && (r3.height>5)) {
            splitRegion(r3,VectorRegion,src,true);
        }
        if(( (maxColorRegion(r4,src) - minColorRegion(r4,src) ) > CRITERE_HOMOGENEITE) && (r4.width> 5) && (r4.height>5)) {
            splitRegion(r4,VectorRegion,src,true);
        }
    }
    
    changement = false;
    return changement;
}

//Fonction qui retourne une nouvelle région qui est la fusion des deux régions passées en paramètre
Region MergeRegion(Region r1, Region r2, Mat &src) {
    Region fusion;
    fusion.x = min(r1.x,r2.x); //coordonnée du point du coin supérieur gauche
    fusion.y = min(r1.y,r2.y); //coordonnée du point du coin supérieur gauche
    fusion.width = max(r1.x + r1.width, r2.x + r2.width)-fusion.x; //coordonnée du point du coin inférieur droit
    fusion.height = max(r1.y + r1.height, r2.y + r2.height)-fusion.y; //coordonnée du point du coin inférieur droit
    fusion.color = colorRegion(fusion,src); //couleur moyenne de la région fusionnée
    return fusion;
}

//Procédure pour dessiner les régions, on parcourt la liste de vecteur et dessiner toutes les régions une à une
void dessinerRegion(Mat &src, vector<Region> VectorRegion, bool color, bool border) {
    srand((unsigned int)time(0));
    int sizevector = VectorRegion.size();
    for(int z = 0; z < sizevector; z++){
        Region r = VectorRegion[z];
        int randomblue = rand() % 255;
        int randomgreen = rand() % 255;
        int randomred = rand() % 255;
        for(int i = r.x ; i < r.x + r.width; i++) {
            for(int j = r.y; j <= r.y + r.height; j++) {
                if(color) {
                    src.at<Vec3b>(j,i)[0] = randomblue; //blue
                    src.at<Vec3b>(j,i)[1] = randomgreen; //green
                    src.at<Vec3b>(j,i)[2] = randomred; //red       
                } else {
                    src.at<Vec3b>(j,i)[0] = r.color;
                    src.at<Vec3b>(j,i)[1] = r.color;
                    src.at<Vec3b>(j,i)[2] = r.color;
                }
            }
        }
        if(border) {
            for(int i = r.x ; i <= r.x + r.width; i++) {
                for(int j = r.y; j <= r.y + r.height; j++) {
                    if(i == r.x || i == r.x + r.width) {
                        src.at<Vec3b>(j,i)[0] = 0;
                        src.at<Vec3b>(j,i)[1] = 0;
                        src.at<Vec3b>(j,i)[2] = 255;
                    }
                    if(j == r.y || j == r.y + r.height) {
                        src.at<Vec3b>(j,i)[0] = 0;
                        src.at<Vec3b>(j,i)[1] = 0;
                        src.at<Vec3b>(j,i)[2] = 255;
                    }
                }
            }
        }
    }
}

int main( int argc, char** argv )
{
    //CommandLineParser parser( argc, argv, "{@input | ../data/TEST1.jpeg | input image}" );
    CommandLineParser parser( argc, argv, "{@input | ../data/TEST2.jpeg | input image}" );
    
    Mat src = imread( samples::findFile( parser.get<String>( "@input" ) ), IMREAD_COLOR );

    if( src.empty() )
    {
        cout << "Could not open or find the image!\n" << endl;
        cout << "Usage: " << argv[0] << " <Input image>" << endl;
        return -1;
    }
    
    //Valeur des dimensions de l'image
    int height = src.size[0];
    int width = src.size[1];

    Mat SplitandMerge;
    SplitandMerge.create(height,width,CV_8UC3);

    Mat Split;
    Split.create(height,width,CV_8UC3);

    //Vecteur qui va stocker les régions
    vector<Region> VectorRegion;
    
    //Région initial (image de base)
    Region initial;
    initial.x = 0;
    initial.y = 0;
    initial.width = width;
    initial.height = height;
    initial.color = colorRegion(initial,src);

    //Ajout de la région initial (image de base) dans le vecteur
    VectorRegion.push_back(initial);

    bool changement = splitRegion(initial,VectorRegion,src,true);
    
    //Affichage de l'image après l'étape de division mais sans la fusion
    dessinerRegion(Split,VectorRegion,false,false);
    imshow("Split",Split);

    //si changement est false c'est qu'il n'y a plus de split en cours donc on peut lancer le processus de fusion
    while(!changement) {
        bool fini = false; //variable pour sortir de la boucle while
        int sizevector = VectorRegion.size();
        for(int i=0 ; i < sizevector; i++) {
            Region r1 = VectorRegion[i];
            for(int j=i+1 ; j < sizevector; j++) {
                Region r2 = VectorRegion[j];
                bool similaire = RegionSimilaire(r1,r2);
                bool adjacent = RegionAdjacent(r1,r2);
                
                if(adjacent && similaire) {
                    Region regionf = MergeRegion(r1,r2,src);
                    VectorRegion.erase(VectorRegion.begin() + j);
                    VectorRegion.erase(VectorRegion.begin() + i);
                    VectorRegion.insert(VectorRegion.begin() + i, regionf);
                    r1 = VectorRegion[i];
                    j--;
                }      
            }
        }

        if (!fini) {
            break;
        }
    }

    //Affichage des différentes images
    dessinerRegion(SplitandMerge,VectorRegion,false,true);
    imshow("SplitandMerge",SplitandMerge);

    imshow("Source",src);

    waitKey();
    return 0;
}

