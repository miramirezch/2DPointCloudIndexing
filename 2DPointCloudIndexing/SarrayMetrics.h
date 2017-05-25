#pragma once

struct SelectRankData{
	
	SelectRankData(unsigned innerProduct, unsigned onesX, unsigned onesY):InnerProd{innerProduct}, OnesPointX{onesX}, OnesPointY{onesY}{}
	unsigned InnerProd;
	unsigned OnesPointX;
	unsigned OnesPointY;
};

SelectRankData InnerProduct(const std::pair<sdsl::sd_vector<>, unsigned> &pointX,const std::pair<sdsl::sd_vector<>, unsigned> &pointY)
{
	// Allow Select
	sdsl::sd_vector<>::select_1_type sarrayX_select(&pointX.first);
	sdsl::sd_vector<>::select_1_type sarrayY_select(&pointY.first);

	// Allow Rank to get the total of 1's bits in Sarray's	
	sdsl::sd_vector<>::rank_1_type sarrayX_rank(&pointX.first);
	sdsl::sd_vector<>::rank_1_type sarrayY_rank(&pointY.first);

	// Total of 1's per Sarray
	auto onesX = sarrayX_rank(pointX.first.size());
	auto onesY = sarrayY_rank(pointY.first.size());  

	// Size of Sarray's
	auto sizeX = pointX.first.size();
	auto sizeY = pointY.first.size();

	unsigned selectX {1};
	unsigned selectY {1};

	//Acumulador - Numero de 1's
	unsigned innerProduct{0};

	// Select 1
	auto indexX = sarrayX_select(selectX);
	auto indexY = sarrayY_select(selectY);

	// Clasic intersection algorithm
	do{		
		if(indexX == indexY){		
			innerProduct+=1;

			selectX +=1;
			selectY +=1;		

			indexX = sarrayX_select(selectX);
			indexY = sarrayY_select(selectY);
		}

		else if (indexX > indexY){
			selectY += 1;			
			indexY = sarrayY_select(selectY);
		}
		else{
			selectX +=1;			
			indexX = sarrayX_select(selectX);
		}

	}while((indexX<sizeX) && (indexY<sizeY) && (selectX <= onesX) && (selectY <= onesY));

	return SelectRankData(innerProduct, onesX, onesY);
}




double CosineSimilarity(const std::pair<sdsl::sd_vector<>, unsigned>& pointX, const std::pair<sdsl::sd_vector<>, unsigned>& pointY) 
{	
	auto selectRankData = InnerProduct(pointX,pointY);
	double similarity = selectRankData.InnerProd/(std::sqrt(selectRankData.OnesPointX)*std::sqrt(selectRankData.OnesPointY));

	// Precision error
	if(similarity >= 1)
	{
		return 0;
	}

	return std::acos(similarity);
}

double HammingDistance(const std::pair<sdsl::sd_vector<>, unsigned>& pointX, const std::pair<sdsl::sd_vector<>, unsigned>& pointY) 
{
	auto selectRankData = InnerProduct(pointX,pointY);	
	return selectRankData.OnesPointX + selectRankData.OnesPointY - (2*selectRankData.InnerProd);	
}

double JaccardSimilarity(const std::pair<sdsl::sd_vector<>, unsigned>& pointX, const std::pair<sdsl::sd_vector<>, unsigned>& pointY) 
{
	auto selectRankData = InnerProduct(pointX,pointY);	
	return 1 - (static_cast<double>(selectRankData.InnerProd) / (selectRankData.OnesPointX + selectRankData.OnesPointY - selectRankData.InnerProd));	
}