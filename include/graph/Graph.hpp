#pragma once

// Dependencies:
// - ranges-v3

// Documentation generator:
// https://jessevdk.github.io/cldoc

// Testing framework:
// https://github.com/catchorg/Catch2

#include <functional>
#include <vector>
#include <optional>

#include "impl/traits.hpp"
#include "impl/Path.hpp"

namespace graph {
	inline namespace v1 {
		namespace impl {
			template <class Impl, class = void>
			struct derivable : Impl {
				template <class... Args>
				explicit derivable(Args&&... args) :
					Impl(std::forward<Args>(args)...) {
				}
			protected:
				const Impl& _impl() const {
					return *this;
				}
				Impl& _impl() {
					return *this;
				}
			};
			template <class Impl>
			struct derivable<Impl,
				std::enable_if_t<std::is_final_v<Impl>>> {
			protected:
				template <class... Args>
				explicit derivable(Args&&... args) :
					_value(std::forward<Args>(args)...) {
				}
				template <class T, class... Args>
				explicit derivable(std::initializer_list<T> il, Args&&... args) :
					_value(il, std::forward<Args>(args)...) {
				}
				const Impl& _impl() const {
					return _value;
				}
				Impl& _impl() {
					return _value;
				}
			private:
				Impl _value;
				operator const Impl&() const {
					return _impl();
				}
			};
		}
		template <class Impl>
		class Graph :
			public impl::derivable<Impl> {
			using _base_type = impl::derivable<Impl>;
		protected:
			using Verts = impl::traits::Verts<Impl>;
			using Edges = impl::traits::Edges<Impl>;
		public:
			using _base_type::_base_type;

			using Vert = typename Verts::value_type;
			using Order = typename Verts::size_type;
			decltype(auto) verts() const {
				return Verts::range(this->_impl());
			}
			Order order() const {
				return Verts::size(this->_impl());
			}
			Vert null_vert() const {
				return Verts::null(this->_impl());
			}
			bool is_null(const Vert& v) const {
				return v == null_vert();
			}

			template <class T> using Vert_map =
				typename Verts::template map_type<T>;
			template <class T>
			Vert_map<T> vert_map(T default_ = {}) const {
				return Verts::map(this->_impl(), std::move(default_));
			} // LCOV_EXCL_LINE (unreachable)
			template <class T> using Ephemeral_vert_map =
				typename Verts::template ephemeral_map_type<T>;
			template <class T>
			Ephemeral_vert_map<T> ephemeral_vert_map(T default_ = {}) const {
				return Verts::ephemeral_map(this->_impl(), std::move(default_));
			}

			using Vert_set = typename Verts::set_type;
			auto vert_set() const {
				return Verts::set(this->_impl());
			}
			using Ephemeral_vert_set = typename Verts::ephemeral_set_type;
			auto ephemeral_vert_set() const {
				return Verts::ephemeral_set(this->_impl());
			}

			using Edge = typename Edges::value_type;
			using Size = typename Edges::size_type;
			decltype(auto) edges() const {
				return Edges::range(this->_impl());
			}
			Size size() const {
				return Edges::size(this->_impl());
			}
			Edge null_edge() const {
				return Edges::null(this->_impl());
			}
			bool is_null(const Edge& e) const {
				return e == null_edge();
			}
			Vert tail(const Edge& e) const {
				return Edges::tail(this->_impl(), e);
			}
			Vert head(const Edge& e) const {
				return Edges::head(this->_impl(), e);
			}

			template <class T> using Edge_map =
				typename Edges::template map_type<T>;
			template <class T>
			Edge_map<T> edge_map(T default_ = {}) const {
				return Edges::map(this->_impl(), std::move(default_));
			} // LCOV_EXCL_LINE (unreachable)
			template <class T> using Ephemeral_edge_map =
				typename Edges::template ephemeral_map_type<T>;
			template <class T>
			Ephemeral_edge_map<T> ephemeral_edge_map(T default_ = {}) const {
				return Edges::ephemeral_map(this->_impl(), std::move(default_));
			}

			using Edge_set = typename Edges::set_type;
			Edge_set edge_set() const {
				return Edges::set(this->_impl());
			}
			using Ephemeral_edge_set = typename Edges::ephemeral_set_type;
			auto ephemeral_edge_set() const {
				return Edges::ephemeral_set(this->_impl());
			}

			using Path = impl::Path<Impl>;
			Path null_path() const { return Path(this->_impl()); }
			template <class Edges = std::vector<Edge>>
			Path path(Vert source, Edges&& edges = {}) const {
				return Path(this->_impl(), std::move(source), std::forward<Edges>(edges));
			}
			Path concatenate_paths(Path&& p0, const Path& p1) const {
				p0._concatenate(this->_impl(), p1);
				return std::move(p0);
			}
			Path concatenate_paths(const Path& p0, const Path& p1) const {
				return concatenate_paths(Path(p0), p1);
			}
			Vert source(const Path& path) const { return path._source(this->_impl()); }
			Vert target(const Path& path) const { return path._target(this->_impl()); }
			bool is_null(const Path& path) const {
				return is_null(source(path));
			}
			bool is_trivial(const Path& path) const {
				return path.is_trivial_or_null() && !is_null(path);
			}

			// Everything below this point should only be declared here and defined in their own .inl files.

			template <class Random>
			Vert random_vert(Random& r) const;

			template <class Random>
			Edge random_edge(Random& r) const;

			/// Construct a view of this graph as its reverse, with reversed edge tails and heads.
			auto reverse_view() const;

			// Constructs a new view of this graph as an empty subforest.
			template <class Adjacency> auto _subforest() const;
			/// Construct a new view of this graph as an empty subforest with edges up to roots.
			auto out_subforest() const { return _subforest<impl::traits::Out>(); }
			/// Construct a new view of this graph as an empty subforest with edges down to leaves.
			auto in_subforest() const { return _subforest<impl::traits::In>(); }

			// Constructs a new view of this graph as an empty subtree.
			template <class Adjacency> auto _subtree(Vert root) const;
			/// Construct a new view of this graph as an empty tree with edges up to a given root.
			auto out_subtree(Vert target) const { return _subtree<impl::traits::Out>(target); }
			/// Construct a new view of this graph as an empty tree with edges down from a given root.
			auto in_subtree(Vert source) const { return _subtree<impl::traits::In>(source); }

			template <class... Args>
			auto dot_format(Args&&...) const;
			template <class... Args>
			auto dot_format(Args&&...);
		};

		template <class Impl>
		Graph(Impl) -> Graph<Impl>;

		// TODO: These should verify that G is a Graph<...> and provide a nice error message otherwise.

		template <class G, class = void>
		struct Traits : Traits<Graph<G>> {
		};

		namespace impl {
			template <class Impl>
			void _is_graph_helper(const Graph<Impl>&);
		}

		template <class G>
		struct Traits<G, decltype(_is_graph_helper(std::declval<G>()))> {
			using Vert = typename G::Vert;
			using Order = typename G::Order;
			template <class T>
			using Vert_map = typename G::template Vert_map<T>;
			using Vert_set = typename G::Vert_set;

			using Edge = typename G::Edge;
			using Size = typename G::Size;
			template <class T>
			using Edge_map = typename G::template Edge_map<T>;
			using Edge_set = typename G::Edge_set;

			using Path = typename G::Path;
		};

		template <class G>
		using Vert = typename Traits<G>::Vert;

		template <class G>
		using Order = typename Traits<G>::Order;

		template <class G, class T>
		using Vert_map = typename Traits<G>::template Vert_map<T>;

		template <class G>
		using Vert_set = typename Traits<G>::Vert_set;

		template <class G>
		using Edge = typename Traits<G>::Edge;

		template <class G>
		using Size = typename Traits<G>::Size;

		template <class G, class T>
		using Edge_map = typename Traits<G>::template Edge_map<T>;

		template <class G>
		using Edge_set = typename Traits<G>::Edge_set;

		template <class G>
		using Path = typename Traits<G>::Path;

		template <class Impl>
		class Out_edge_graph :
			public virtual Graph<Impl> {
			using _base_type = Graph<Impl>;
			using Out_edges = impl::traits::Out_edges<Impl>;
		public:
			using _base_type::_base_type;

			using Vert = typename _base_type::Vert;
			using Edge = typename _base_type::Edge;
			using Out_degree = typename Out_edges::size_type;
			decltype(auto) out_edges(const Vert& v) const {
				return Out_edges::range(this->_impl(), v);
			}
			Out_degree out_degree(const Vert& v) const {
				return Out_edges::size(this->_impl(), v);
			}

			template <class Weight, class Compare = std::less<>, class Combine = std::plus<>>
			auto shortest_paths_from(const Vert& s, const Weight& weight,
				const Compare& compare = {}, const Combine& combine = {}) const;

			auto reverse_view() const;
			// auto scc() const;

			template <class WM, class Compare = std::less<>>
			auto minimum_tree_reachable_from(const Vert& s, const WM& weight, const Compare& compare = {}) const;
		};

		template <class Impl>
		Out_edge_graph(Impl) -> Out_edge_graph<Impl>;

		template <class Impl>
		class In_edge_graph :
			public virtual Graph<Impl> {
			using _base_type = Graph<Impl>;
			using In_edges = impl::traits::In_edges<Impl>;
		public:
			using _base_type::_base_type;

			using Vert = typename _base_type::Vert;
			using Edge = typename _base_type::Edge;
			using In_degree = typename In_edges::size_type;
			decltype(auto) in_edges(const Vert& v) const {
				return In_edges::range(this->_impl(), v);
			}
			In_degree in_degree(const Vert& v) const {
				return In_edges::size(this->_impl(), v);
			}

			template <class Weight, class Compare = std::less<>, class Combine = std::plus<>>
			auto shortest_paths_to(const Vert& t, const Weight& weight,
				const Compare& compare = {}, const Combine& combine = {}) const;

			auto reverse_view() const;
			// auto scc() const;

			template <class WM, class Compare = std::less<>>
			auto minimum_tree_reaching_to(const Vert& t, const WM& weight, const Compare& compare = {}) const;
		};

		template <class Impl>
		In_edge_graph(Impl) -> In_edge_graph<Impl>;

		template <class Impl>
		class Bi_edge_graph :
			public Out_edge_graph<Impl>,
			public In_edge_graph<Impl> {
			using _base_type = Graph<Impl>;
		public:
			using Vert = typename _base_type::Vert;
			using Edge = typename _base_type::Edge;
			using Path = typename _base_type::Path;
			// using _base_type::_base_type;
			template <class... Args,
				class = std::enable_if_t<std::is_constructible_v<_base_type, Args&&...>>>
			Bi_edge_graph(Args&&... args) :
				_base_type(std::forward<Args>(args)...) {
			}

			template <class WM, class Compare = std::less<>, class Combine = std::plus<>>
			auto shortest_path(const Vert& s, const Vert& t, const WM& weight,
				const Compare& compare = {}, const Combine& combine = {}) const -> Path;
			template <class WM, class Compare = std::less<>, class Combine = std::plus<>>
			auto parallel_shortest_path(const Vert& s, const Vert& t, const WM& weight,
				const Compare& compare = {}, const Combine& combine = {}) const -> Path;
		};

		template <class Impl>
		Bi_edge_graph(Impl) -> Bi_edge_graph<Impl>;

		template <class Impl>
		class Graph<Graph<Impl>> {
			static_assert(sizeof(Impl) == 0,
				"internal error in graph library");
		};

		template <class Impl>
		class Graph<Out_edge_graph<Impl>> : Graph<Graph<Impl>> {};

		template <class Impl>
		class Graph<In_edge_graph<Impl>> : Graph<Graph<Impl>> {};

		template <class Impl>
		class Graph<Bi_edge_graph<Impl>> : Graph<Graph<Impl>> {};

		template <class Impl>
		auto _wrap_graph(Impl&& impl) {
			constexpr bool
				out = impl::traits::is_out_edge_graph_v<Impl>,
				in = impl::traits::is_in_edge_graph_v<Impl>;
			using Wrapper =
				std::conditional_t<out && in, Bi_edge_graph<Impl>,
				std::conditional_t<out, Out_edge_graph<Impl>,
				std::conditional_t<in, In_edge_graph<Impl>,
				Graph<Impl>>>>;
			return Wrapper(std::forward<Impl>(impl));
		}
	}
}

// Inlines
#include "dijkstra.inl"
#include "random.inl"
#include "reverse.inl"
#include "subforest.inl"
//#include "scc.inl"
#include "bidirectional_search.inl"
#include "parallel_bidirectional_search.inl"
#include "format.inl"
