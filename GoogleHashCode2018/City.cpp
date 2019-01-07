#include"City.h"
#include "Project.h"
using namespace std;
PlacedBuilding::PlacedBuilding(const PlacedBuilding &P)
{
	position = P.position;
	if(P.source->getType()==Residential)
	{
		for (auto pair : Project::globalProject.utilities)
		{
			connectedUtility[pair.first] = 0;
		}
	}
	source = P.source;
}
	PlacedBuilding::PlacedBuilding(const Building *buildingNum)
{
	this->source = buildingNum;
	if (buildingNum->getType() == Residential)
	{
		for (auto i : Project::globalProject.utilities)
		{
			connectedUtility[i.first] = 0;
		}
	}
}
PlacedBuilding::PlacedBuilding(PlacedBuilding &P, Coord C)
{
	position = P.position + C;
	if (P.source->getType() == Residential)
	{
		for (auto pair : P.connectedUtility)
		{
			connectedUtility[pair.first] = (pair.second);
		}
	}
	source = P.source;
}
/**
 * @brief
 * Test if a utility type is already use by a residential
 * In this case we return a score equal to the extra of the residential
 * 	and set this utility type as use for this placed building
 * Else we return 0
 * @param utilityType
 * @return int Score generated by the use of this utility type
 */
int PlacedBuilding::use(unsigned int utilityType)
{
	if (connectedUtility[utilityType] == 0)
	{
		connectedUtility[utilityType] += 1;
		return source->getExtra();
	}
	return 0;
}
int PlacedBuilding::undo()
{
	int out = 0;
	switch(source->getType())
	{
		case Residential:
			for(auto it:connectedUtility)
			{
				if(it.second>0)
				{
					out+= source->getExtra();
				}
			}
			break;
	}
}

City::City() {
	RemainingCellsList = set<Coord>();
}

City::City(unsigned int w, unsigned int h)
{
	RemainingCellsList = set<Coord>();
	this->width = w;
	this->height = h;

	buildingMap = vector<vector<short int>>(height);
	fill(buildingMap.begin(),buildingMap.end(),vector<short int>(width));
	for(auto& v:buildingMap)
		fill(v.begin(),v.end(),-1);

	for (unsigned int i = 0; i < getCityHeight(); i++)
	{
		for (unsigned int j = 0; j < getCityWidth(); j++)
		{
			RemainingCellsList.insert({short(i), short(j)});
		}
	}
	score = 0;
}
/**
 * @brief Construct a new City:: City object
 * Copy constructor
 * @param c 
 */
City::City(const City& c)
{
	width=c.width;
	height=c.height;
	buildingMap = c.buildingMap;
	RemainingCellsList = c.RemainingCellsList;
	for (PlacedBuilding P : c.placedBuildingRegister)
	{
		placedBuildingRegister.push_back(P);
		switch (P.source->getType())
		{
		case Residential:
			registeredResidentials.push_back(&placedBuildingRegister.back());
			break;
		case Utility:
			registeredUtilities.push_back(&placedBuildingRegister.back());
			break;
		}
	}
	score = c.score;
}
/**
 * @brief
 * Assignation operator
 * @param c 
 * @return City&
 */
City& City::operator=(City& c)
{
	width = c.width;
	height = c.height;
	buildingMap = c.buildingMap;
	RemainingCellsList = c.RemainingCellsList;

	for(PlacedBuilding P:c.placedBuildingRegister)
	{
		placedBuildingRegister.push_back(P);
		switch(P.source->getType())
		{
			case Residential:
				registeredResidentials.push_back(&placedBuildingRegister.back());
			break;
			case Utility:
				registeredUtilities.push_back(&placedBuildingRegister.back());
				break;
		}
	}
	score = c.score;
	return *this;
}
/**
 * @brief Construct a new City object from an other
 *
 * @param h
 * @param w
 * @param c 
 * @param row
 * @param col 
 */
City::City(unsigned int h, unsigned w, City& c, unsigned int row, unsigned int col)
{
	this->width = w;
	this->height = h;
	buildingMap = vector<vector<short int>>(height);
	fill(buildingMap.begin(), buildingMap.end(), vector<short int>(width));
	for (auto &v : buildingMap)
		fill(v.begin(), v.end(), -1);
	placeMap(c,row,col);
}
/**
 * @brief
 * Insert a city in an other bigger
 * @param c
 * @param row 
 * @param col
 * @return true
 * @return false
 */
bool City::placeMap( City &c, unsigned int row, unsigned int col)
{

	int i =0;
	for (PlacedBuilding& P : c.placedBuildingRegister)
	{
		if(!placeBuilding(P.source,P.position.row+row,P.position.column+col,false))
			return false;
	}
	return true;
}
	/*
	Place a building on the buildingMap
	The try argument determine if it has to test the placement before placing the building.
	if it's 0, this mean an error occured,
	else the closest it is from 1, the more the placement is optimized.
*/
double City::placeBuilding(const Building *building, unsigned int row, unsigned int col, bool test)
{
	int num = this->placedBuildingRegister.size();
	unsigned int row_temp;
	unsigned int col_temp;
	//Unité de mesure du placement
	double coverage = 0;
	int maxCoverage = building->getColumnNum() * building->getRowNum();
	bool stop = false;
	// On check les cellules que prend le building
	if(test)
	{
		if(building->getColumnNum()+col<=width && building->getRowNum()+row<=height)
		{
			for(auto &c : building->getCases())
			{
				if (this->getMapCell(c.row+row, c.column+col) != -1)
				{
					return 0;
					//Cas du chevauchement
				}
			}
		}
		else
		{
			return 0;
		}
	}
	for (auto &c : building->getCases())
	{
			if(connexInit)
			{
				ConnexComposant[connexMap[c.row + row][c.column + col]].erase({short(c.row + row), short(c.column + col)});
			}
			RemainingCellsList.erase({ short(c.row+row),short(c.column+col)});
			this->setMapCell(c.row+row, c.column+col, num);
			coverage++; //Cas du chevauchement
	}
	PlacedBuilding placedBuilding(building);
	placedBuilding.position = Coord(row, col);
	placedBuildingRegister.push_back(placedBuilding);
	switch (building->getType())
	{
	case Building_type::Residential:
		registeredResidentials.push_back(&placedBuildingRegister.back());
		break;
	case Building_type::Utility:
		registeredUtilities.push_back(&placedBuildingRegister.back());
		break;
	}
	//Calcul du score généré par le placement
	for (const Coord &coord : placedBuilding.source->getInfluenceArea())
	{
		Coord temp_coord = {short(coord.row + int(row)), short(coord.column + int(col))};
		if (temp_coord.row >= 0 && temp_coord.row < int(height) && temp_coord.column >= 0 && temp_coord.column < int(width) && getMapCell(temp_coord.row, temp_coord.column) > -1 && &placedBuildingRegister[getMapCell(temp_coord.row, temp_coord.column)] != &placedBuildingRegister.back())
		{
			score += computeScore(placedBuildingRegister.back(), placedBuildingRegister[getMapCell(temp_coord.row, temp_coord.column)]);
		}
	}
	return coverage / maxCoverage;
}
/**
 * @brief
 *	return the list of remaining cells as a list of Coord.
 * @return set<Coord>
 */
const set<Coord>& City::getRemainingCellsList() const
{
	return RemainingCellsList;
}
/**
 * @brief
 * Get and create if necessary de connex composant liste
 * Which is all cells that are in an area in which we can place a building
 * @return vector<set<Coord>>
 */
vector<set<Coord>>&& City::getConnexComposant()
{
	if(!connexInit ||connexCount++>=connexResetFrequency)
	{
	connexCount =0;
	map<int, set<Coord>> ConnexComposant;
	int counter = 0;
	connexMap = vector<vector<short int>>(height);
	fill(connexMap.begin(), connexMap.end(), vector<short int>(width));
	for (auto v : connexMap)
		fill(v.begin(), v.end(), -1);
	for(auto C : RemainingCellsList)
	{
		bool alreadyPlaced = false;
		int value = -1;
		for(int row=-1;row<=1;row++)
		{
			for(int col=-1;col<=1;col++)
			{
				if(row+int(C.row)>=0 && row+int(C.row)<height && col+int(C.column)>=0 && col+int(C.column)<width && col!=row)
				{

						int CasesVal = connexMap[row + C.row][col + C.column] ;
						if (!alreadyPlaced && CasesVal >= 0)
						{
							alreadyPlaced = true;
							value = CasesVal;
							connexMap[C.row][C.column] = CasesVal;
							ConnexComposant[value].insert({C.row,C.column});
						}
						else if (alreadyPlaced && CasesVal != value &&CasesVal>=0)
						{
							for (auto B : ConnexComposant[CasesVal])
							{
								connexMap[B.row][B.column] = value;
								ConnexComposant[value].insert(B);
								ConnexComposant[CasesVal].erase(B);
							}
							ConnexComposant.erase(CasesVal);
						}

				}
			}
		}
		if(!alreadyPlaced)
		{

			connexMap[C.row][C.column] = counter;
			ConnexComposant[counter] = set<Coord>();
			ConnexComposant[counter].insert(C);
			counter++;
		}
	}
	connexInit = true;
	}
	vector<set<Coord>> outVec;
	for(auto C:ConnexComposant)
	{
		if(C.second.size() >= Project::globalProject.minNbCells)
		{
			//random_shuffle(C.second.begin(),C.second.end());
			outVec.push_back(C.second);
		}
	}
	sort(outVec.begin(), outVec.end(), [](const set<Coord> &a, const set<Coord> &b)
	{
		return a.size() < b.size();
	});
		//random_shuffle(outVec.begin(),outVec.end());
		return move(outVec);
}
/**
 * @brief
 * Return the number of remaining cells.
 * @return int 
 */
int City::getRemainingCell() const
{
	return RemainingCellsList.size();
}
int City::getBuildingQuantity() const
{
	return placedBuildingRegister.size();
}
		/**
 * @brief
 * Compute the score generated by a freshly placed building A with an other buildbing B accesible by A.
 * @param A Freshly placed building
 * @param B Building already on the buildingMap
 * @return int Score generated by the interaction between these 2 buildings.
 */
		int City::computeScore(PlacedBuilding & A, PlacedBuilding & B)
		{
			switch (A.source->getType())
			{
			case Utility:
				switch (B.source->getType())
				{
				case Residential:
					return B.use(A.source->getExtra());
				break;
				default:
					return 0;
				break;
			}
		break;
		case Residential:
			switch (B.source->getType())
			{
			case Utility:
				return A.use(B.source->getExtra());
				break;
			default:
				return 0;
				break;
			}
			break;
	}
	return 0;
}
/**
 * @brief
 * Print the city buildingMap in the console
 */
void City::PrintMap()
{
	for(int r = 0;r<height;r++)
	{
		for(int c=0;c<width;c++)
		{
			if(buildingMap[r][c]>=0)
				cout  << "#";
			else
				cout << ".";
		}
		cout << endl;
	}
}
/**
 * @brief
 *	Remove the last nbBuilding placed on the city.
 * @param nbBUilding
 *
 */
void City::undo(int nbBUilding)
{
	while(nbBUilding-->0)
	{
		undo();
	}
}
/**
 * @brief
 * Remove the last placed building.
 */
void City::undo()
{
	auto& lastB = placedBuildingRegister.back();
	int out = 0;
	switch (lastB.source->getType())
	{
		case Residential:
		for (auto it : lastB.connectedUtility)
		{
			if (it.second > 0)
			{
				out += lastB.source->getExtra();
			}
		}
		registeredResidentials.pop_back();
		break;
		case Utility:
			//Calcul du score généré par le placement
			map<int,bool> alreadyDone;
			for (const Coord &coord : lastB.source->getInfluenceArea())
			{
				Coord temp_coord = coord + lastB.position;
				if (temp_coord.row >= 0 && temp_coord.row < int(height) && temp_coord.column >= 0 && temp_coord.column < int(width) && getMapCell(temp_coord.row, temp_coord.column) > -1 && &placedBuildingRegister[getMapCell(temp_coord.row, temp_coord.column)] != &placedBuildingRegister.back())
				{
					short int buildingNumber = getMapCell(temp_coord.row, temp_coord.column);
					if (alreadyDone.find(buildingNumber) == alreadyDone.end())
					{
						alreadyDone[buildingNumber] = true;
						auto &build = placedBuildingRegister[buildingNumber];
						build.connectedUtility[lastB.source->getExtra()]--;
						if(build.connectedUtility[lastB.source->getExtra()]<=0)
						{
							out += build.source->getExtra();
						}
					}
				}
			}
			registeredUtilities.pop_back();
			break;
	}
	for(const auto& C:lastB.source->getCases())
	{
		Coord temp = C + lastB.position;
		setMapCell(temp.row,temp.column,-1);
		RemainingCellsList.insert(C + lastB.position);
	}
	score-= out;
	placedBuildingRegister.pop_back();
}

/*
	Save the city into a file
*/
 void City::toSolution(string outfileName)
 {
	 ofstream outfile(outfileName);
	 outfile << placedBuildingRegister.size() << endl;
	 for (int i = 0; i < placedBuildingRegister.size(); i++)
	 {
		 outfile << placedBuildingRegister.at(i).source->getProjectNum() << " " << placedBuildingRegister.at(i).position.row << " " << placedBuildingRegister.at(i).position.column << endl;
	 }
	 outfile.close();
 }


/*
	Modify the value of the buildingMap's cell in parameter
*/
void City::setMapCell(int x, int y, int value)
{
	this->buildingMap[x][y] = value;
}

/*
	Return the value of the buildingMap's cell in parameter
*/
short int City::getMapCell(int x, int y) const
{
	return this->buildingMap[x][y];
}
/**
 * @brief
 * Return the current socre of the city.
 * @return int 
 */
int City::getScore() const
{
	return score;
}

Coord operator+(const Coord& A,const Coord& B)
{
	return {A.row+B.row,A.column+B.column};
}

bool operator<(const Coord A, const Coord B)
{
	if (A.column < B.column)
	{
		return true;
	}
	else if (A.column > B.column)
	{
		return false;
	}
	else
	{
		if (A.row < B.row)
			return true;
		else
			return false;
	}
}
bool operator==(const Coord &A, const Coord &B)
{
	return A.row == B.row && A.column == B.column;
}