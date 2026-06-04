/*
 * StateReco.hpp
 *
 *  Created on: May 21, 2026
 *      Author: Local user
 */
#include <cstdint>
#include "RocketStates.hpp"
#include "actualflash.hpp"

#ifndef FLIGHTCONTROL_INC_STATERECO_HPP_
#define FLIGHTCONTROL_INC_STATERECO_HPP_

#define STATE_RECO_INVALID_IDX (sectorCount+1)
#define STATE_RECO_INVALID_GEN (0xffffffff)
#define STATE_RECO_SECTOR_START (0x00800000 / 4096)
#define STATE_RECO_SECTOR_COUNT (2)

class StateRecoverer {
public:
	static StateRecoverer& Inst() {
		static StateRecoverer s(STATE_RECO_SECTOR_START,STATE_RECO_SECTOR_COUNT);
		return s;
	}

	StateRecoverer(uint32_t sectorStart, uint32_t sectorCount) : sectorStart(sectorStart), sectorCount(sectorCount) {
		MX66L1G45G::Inst().Init();
		RecoSectorIndex recent = GetMostRecentValid();
		if(recent == STATE_RECO_INVALID_IDX) {
			generation = 1;
		} else {
			generation = GetSave(recent).gen + 1;

		}

	}

	RocketState GetMostRecentState();

	bool SaveState(RocketState state);

private:

	typedef uint32_t RecoSectorIndex;
	const uint32_t sectorStart;
	const uint32_t sectorCount;
	uint32_t generation;

	struct StateSave {
		RocketState state;
		uint32_t tick;
		uint32_t gen;
		uint32_t checksum;
	};

	uint32_t GetChecksum(StateSave save) const;
	RecoSectorIndex GetMostRecentValid();
	StateSave GetSave(RecoSectorIndex index);
	bool PutSave(RecoSectorIndex index, StateSave state);


};



#endif /* FLIGHTCONTROL_INC_STATERECO_HPP_ */
