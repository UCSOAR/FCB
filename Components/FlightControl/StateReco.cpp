
/*
 * StateReco.cpp
 *
 *  Created on: May 21, 2026
 *      Author: Local user
 */


#include "StateReco.hpp"



RocketState StateRecoverer::GetMostRecentState() {

	RecoSectorIndex i = GetMostRecentValid();
	if(i == STATE_RECO_INVALID_IDX) {
		return RocketState::RS_NONE;
	}
	return GetSave(i).state;

}

bool StateRecoverer::SaveState(RocketState state) {
	RecoSectorIndex lastValid = GetMostRecentValid();
	RecoSectorIndex toWrite;
	if(lastValid == STATE_RECO_INVALID_IDX) {
		toWrite = 0;
	} else {
		toWrite = (lastValid+1)%sectorCount;
	}

	StateSave s;
	s.state = state;
	s.tick = HAL_GetTick();
	s.gen = generation;
	s.checksum = GetChecksum(s);

	generation++;

	return PutSave(toWrite, s);
}

uint32_t StateRecoverer::GetChecksum(StateSave save) const {
	// the specifics of this dont matter as long as its unique
	uint32_t buf[] = {save.state,save.tick,save.gen};
	return HAL_CRC_Calculate(&hcrc, &buf[0], sizeof(buf));
	//return ((((((uint32_t)save.state) << 10) ^ (save.tick)))*5 + 1)*5 + save.gen*123;
}

StateRecoverer::RecoSectorIndex StateRecoverer::GetMostRecentValid() {
	StateSave highestSave = {RocketState::RS_NONE,0,0,0};
	RecoSectorIndex highestidx;
	bool foundValid = false;
	for(RecoSectorIndex i = 0; i <sectorCount; i++) {
		StateSave thisSave = GetSave(i);

		if(thisSave.gen == 0xffffffff) {
			continue;
		}

		if(thisSave.checksum == GetChecksum(thisSave)) {
			if(thisSave.gen > highestSave.gen ||
					(thisSave.gen == highestSave.gen && thisSave.tick > highestSave.tick) || !foundValid) {
				highestSave = thisSave;
				highestidx = i;
			}
			foundValid = true;
		}
	}

	if(foundValid)
		return highestidx;
	else
		return STATE_RECO_INVALID_IDX;
}

StateRecoverer::StateSave StateRecoverer::GetSave(RecoSectorIndex index) {

	StateSave s;
	if(index >= sectorCount || !MX66L1G45G::Inst().ReadData((sectorStart+index)*4096, (uint8_t*)&s, sizeof(s))) {
		return {RocketState::RS_NONE,0,0,0};
	}
	return s;
}

bool StateRecoverer::PutSave(RecoSectorIndex index, StateSave state) {
	if(index >= sectorCount || !MX66L1G45G::Inst().EraseSector((sectorStart+index)*4096)) {
		return false;
	}

	return MX66L1G45G::Inst().WriteData((sectorStart+index)*4096,
			(uint8_t*)&state,
			sizeof(state));
}
