#pragma once

#include "../Solver.hpp"

namespace Cluedo {

namespace UI {

class PlayerDataModal {
public:
	explicit PlayerDataModal() = default;

	void show(Solver const&);
};

}

}
