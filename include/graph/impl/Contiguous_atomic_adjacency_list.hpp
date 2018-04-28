#pragma once

#include <atomic>

#include <range/v3/view/all.hpp>
#include <range/v3/view/iota.hpp>
#include <range/v3/view/transform.hpp>

#include "construct_fn.hpp"
#include "exceptions.hpp"
#include "integral_wrapper.hpp"
#include "contiguous_key_map.hpp"
#include "unordered_set.hpp"
#include "atomic_list.hpp"

namespace graph {
	inline namespace v1 {
		namespace impl {
			template <class Order_ = std::size_t>
			struct Contiguous_atomic_vert_list {
				using Order = Order_;
				using Vert = integral_wrapper<Order, struct vert_tag>;
				auto verts() const {
					return ranges::view::iota(Order{}, order()) |
						ranges::view::transform(construct<Vert>);
				}
				auto null_vert() const noexcept {
					return Vert{};
				}
				auto order() const noexcept {
					return _vlast.load();
				}
				void reserve_verts(Order capacity) {
					_vcapacity = std::max({_vcapacity, capacity});
				}
				Order vert_capacity() const noexcept {
					return _vcapacity;
				}
				auto insert_vert() {
					auto vk = _vlast++;
					check_precondition(vk < vert_capacity(), "insufficient vertex capacity");
					return Vert{vk};
				}
				template <class T>
				using Vert_map = persistent_contiguous_key_map<Vert, T>;
				template <class T>
				auto vert_map(T default_) const {
					return Vert_map<T>(_vcapacity, std::move(default_));
				}
				template <class T>
				using Ephemeral_vert_map = ephemeral_contiguous_key_map<Vert, T>;
				template <class T>
				auto ephemeral_vert_map(T default_) const {
					return Ephemeral_vert_map<T>(order(), std::move(default_));
				}

				using Vert_set = unordered_set<Vert>;
				auto vert_set() const {
					return Vert_set();
				}
				//using Ephemeral_vert_set = Vert_set;
				using Ephemeral_vert_set = ephemeral_contiguous_key_set<Vert>;
				auto ephemeral_vert_set() const {
					//return vert_set();
					return Ephemeral_vert_set(order());
				}
			private:
				std::atomic<Order> _vlast = 0;
				Order _vcapacity = 0;
			};

			template <class Order_ = std::size_t, class Size_ = std::size_t>
			struct Contiguous_atomic_edge_list :
				Contiguous_atomic_vert_list<Order_> {
				using _base_type = Contiguous_atomic_vert_list<Order_>;
				using Vert = typename _base_type::Vert;
				using Size = Size_;
				using Edge = integral_wrapper<Size, struct edge_tag>;
				auto edges() const {
					return ranges::view::iota(Size{}, size()) |
						ranges::view::transform(construct<Edge>);
				}
				auto null_edge() const noexcept {
					return Edge{};
				}
				auto size() const noexcept {
					return _elast.load();
				}
				auto tail(const Edge& e) const {
					return _elist[e.key()].first;
				}
				auto head(const Edge& e) const {
					return _elist[e.key()].second;
				}
				void reserve_edges(Size capacity) {
					_elist.resize(std::max({_elist.size(), capacity}));
				}
				Size edge_capacity() const noexcept {
					return _elist.size();
				}
				auto insert_edge(Vert s, Vert t) {
					auto ek = _elast++;
					check_precondition(ek < edge_capacity(), "insufficient edge capacity");
					_elist[ek] = std::make_pair(s, t);
					return Edge(ek);
				}
				template <class T>
				using Edge_map = persistent_contiguous_key_map<Edge, T>;
				template <class T>
				auto edge_map(T default_) const {
					return Edge_map<T>(_elist.size(), std::move(default_));
				} // LCOV_EXCL_LINE (unreachable)
				template <class T>
				using Ephemeral_edge_map = ephemeral_contiguous_key_map<Edge, T>;
				template <class T>
				auto ephemeral_edge_map(T default_) const {
					return Ephemeral_edge_map<T>(size(), std::move(default_));
				}

				using Edge_set = unordered_set<Edge>;
				auto edge_set() const {
					return Edge_set();
				}
				//using Ephemeral_edge_set = Edge_set;
				using Ephemeral_edge_set = ephemeral_contiguous_key_set<Edge>;
				auto ephemeral_edge_set() const {
					//return edge_set();
					return Ephemeral_edge_set(size());
				}
			private:
				std::vector<std::pair<Vert, Vert>> _elist;
				std::atomic<Size> _elast = 0;
			};

			template <class Adjacency, template <class> class Atomic_container = atomic_list>
			struct Contiguous_atomic_adjacency_list_base :
				Contiguous_atomic_edge_list<> {
				using _base_type = Contiguous_atomic_edge_list<>;

				using Order = typename _base_type::Order;
				using Vert = typename _base_type::Vert;

				using _alist_type = Atomic_container<Edge>;
				using _vlist_type = std::vector<_alist_type>;
				using _degree_type = typename _alist_type::size_type;

				void reserve_verts(Order capacity) {
					_base_type::reserve_verts(capacity);
					_vlist.resize(this->vert_capacity());
				}
				auto insert_edge(Vert s, Vert t) {
					auto e = _base_type::insert_edge(s, t);
					static_assert(std::is_same_v<Adjacency, traits::Out> || std::is_same_v<Adjacency, traits::In>);
					if constexpr (std::is_same_v<Adjacency, traits::Out>)
						_vlist[s.key()].emplace(e);
					else
						_vlist[t.key()].emplace(e);
					return e;
				}
				auto _vert_edges(Vert k) const {
					return ranges::view::all(_vlist[k.key()]);
				}
				auto _vert_degree(Vert k) const {
					return _vlist[k.key()].conservative_size();
				}
			private:
				_vlist_type _vlist;
			};

			struct Contiguous_atomic_out_adjacency_list :
				Contiguous_atomic_adjacency_list_base<traits::Out> {
				using _base_type = Contiguous_atomic_adjacency_list_base<traits::Out>;
				using _base_type::_base_type;
				using Vert = typename _base_type::Vert;
				using Edge = typename _base_type::Edge;
				using Out_degree = typename _base_type::_degree_type;
				inline auto out_edges(const Vert& v) const {
					return _base_type::_vert_edges(v);
				}
				inline Out_degree out_degree(const Vert& v) const {
					return _base_type::_vert_degree(v);
				}
			};

			struct Contiguous_atomic_in_adjacency_list :
				Contiguous_atomic_adjacency_list_base<traits::In> {
				using _base_type = Contiguous_atomic_adjacency_list_base<traits::In>;
				using _base_type::_base_type;
				using Vert = typename _base_type::Vert;
				using Edge = typename _base_type::Edge;
				using In_degree = typename _base_type::_degree_type;
				inline auto in_edges(const Vert& v) const {
					return _base_type::_vert_edges(v);
				}
				inline In_degree in_degree(const Vert& v) const {
					return _base_type::_vert_degree(v);
				}
			};
		}
	}
}

#if GRAPH_V1_IMPL_ATOMIC_ADJACENCY_LIST_USE_CONTIGUOUS_MAPS
namespace std {
	template <class T>
	struct hash<::graph::v1::impl::node_pointer_wrapper<T>> {
		using argument_type = ::graph::v1::impl::node_pointer_wrapper<T>;
		auto operator()(const argument_type& a) const {
			// We have a couple of options here.  We could hash the pointer or we could hash the key, since both are unique.  The key produces fewer collisions but requires an extra indirection, so doesn't seem to be worthwhile.
			return _inner_hash(a._p);
		}
	private:
		using _inner_hash_type = hash<T *>;
		_inner_hash_type _inner_hash;
	};
}
#endif