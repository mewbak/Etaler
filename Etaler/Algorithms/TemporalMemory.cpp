#include "TemporalMemory.hpp"

using namespace et;

TemporalMemory::TemporalMemory(const Shape& input_shape, size_t cells_per_column, size_t max_synapses_per_cell, Backend* backend)
	:input_shape_(input_shape)
{
	Shape connection_shape = input_shape + cells_per_column + max_synapses_per_cell;

	connections_ = constant(connection_shape, -1, backend);
	permances_ = Tensor(connection_shape, DType::Float, backend);
}

std::pair<Tensor, Tensor> TemporalMemory::compute(const Tensor& x, const Tensor& last_state)
{
	et_assert(x.shape() == input_shape_);
	Tensor active_cells;
	if(last_state.has_value() == true)
		active_cells = burst(x, last_state);
	else
		active_cells = burst(x, zeros(x.shape()+cellsPerColumn(), DType::Bool, x.backend()));
	Tensor activity = cellActivity(active_cells, connections_, permances_, connected_permance_, active_threshold_);
	Tensor predictive_cells = cast(activity, DType::Bool);

	return {predictive_cells, active_cells};

}

void TemporalMemory::learn(const Tensor& active_cells, const Tensor& last_active)
{
	Tensor learning_cells = reverseBurst(active_cells);

	learnCorrilation(last_active, learning_cells, connections_, permances_, permance_inc_, permance_dec_);
	growSynapses(last_active, learning_cells, connections_, permances_, 0.21);

}