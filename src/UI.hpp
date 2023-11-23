#pragma once

#include "Solver.hpp"
#include <ftxui/component/component.hpp>

namespace ftxui {

class ScrollerBase : public ComponentBase {
public:
	ScrollerBase(Component child) { Add(child); }

private:
	bool Focusable() const override { return true; }
	Element Render() override;
	bool OnEvent(Event event) override;

	int m_selected = 0;
	int m_size = 0;
	Box m_box;
};

Component Scroller(Component child);

}

namespace Cluedo {

namespace UI {

template<typename ValueType>
ftxui::Component NumberInput(ValueType& value, std::optional<ValueType> minimum = {}, std::optional<ValueType> maximum = {}, std::optional<std::function<void()>> on_change = {});

Solver create_solver();
void main();

};

};
