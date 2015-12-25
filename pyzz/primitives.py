from .pyzz import *


def _extend_to_power_of_2(wires, w):
    # can be done faster, but its not worth the cryptic code
    for next_power_of_two in xrange(len(wires)):
        if 1<<next_power_of_two >= len(wires):
            next_power_of_two = 1<<next_power_of_two
            break
    wires.extend( [w] * (next_power_of_two - len(wires)))


def compare_and_swap_wires(w1, w2):
    return w1&w2, w1|w2


def mux_n(w_control, v_then, v_else):
    assert len(v_then) == len(v_else)
    return [w_control.ite(t, e) for t, e in zip(v_then, v_else)]


def less_than_or_equals_n(N, v1, v2):
    assert len(v1) == len(v2)
    if not v1:
        return N.get_True()
    return ~v1[0]&v2[0] | v1[0].equals(v2[0])&less_than_or_equals(N, v1[1:], v2[1:])


def compare_and_swap_n(v1, v2):
    lte = less_than_or_equals_n(v1, v2)
    return mux_n(lte, v1, v2), mux_n(lte, v2, v1),


def batcher_sorting_network(N, wires, compare_and_swap=compare_and_swap_wires):
    "a 0-1 Batcher sorting network"

    def do_compare_and_swap(i, j):
        V[i], V[j] = compare_and_swap(V[i], V[j])

    def merge(indices):

        if len(indices)==2:
            do_compare_and_swap(*indices)
            return

        merge(indices[0::2])
        merge(indices[1::2])

        for i in xrange(1, len(indices)-1, 2):
            do_compare_and_swap(indices[i], indices[i+1])

    def sort(indices):

        assert len(indices)%2 == 0

        if len(indices) == 2:
            do_compare_and_swap(*indices)
            return

        left = indices[:len(indices)/2]
        right = indices[len(indices)/2:]

        sort(left)
        sort(right)

        merge(left + right)

    V = list(wires)
    _extend_to_power_of_2(V, N.get_True())
    sort(range(0, len(V)))
    return V[:len(wires)]
