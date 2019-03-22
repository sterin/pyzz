# requires click by Armin Ronacher (pip install click)

import click
import pyzz


@click.command()
@click.argument("aig", type=click.Path(exists=True, dir_okay=False))
@click.option("--max", type=int, default=25)
def bmc(aig, max):

    N = pyzz.netlist.read_aiger(aig)
    N.remove_unreach()

    symbols = pyzz.utils.make_symbols(N)

    pyzz.bmc.safety_bmc(N, max, symbols)


if __name__ == "__main__":
    bmc()
