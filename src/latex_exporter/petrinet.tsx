import { h, Node, Edge, Diagram } from 'jsx-tikzcd';
import { IPetriNet, IPlaceNode, IEdge, INode } from '../petri_nets';
import { PlaceRow } from './places';
import { TransitionRow } from './transitions';
import { xml } from 'd3';

interface IPlaceNodesProps {
  readonly places: IPlaceNode[][];
  readonly columns: number;
  readonly interlace: boolean;
};

interface ITransitionNodesProps {
  readonly transitions: INode[][];
  readonly columns: number;
  readonly interlace: boolean;
};

interface ILatexNodesProps {
  readonly places: IPlaceNode[][];
  readonly transitions: INode[][];
  readonly columns: number;
  readonly interlace: boolean;
};

interface IEdgesProps {
  readonly edges: IEdge[];
  readonly type: string;
};

interface ILatexPetriNetProps {
  readonly graph: IPetriNet;
};

interface IMaximumResult {
  readonly index: number;
  readonly value: number;
};

function findMaximumLength<T>(matrix: T[][]): IMaximumResult {
  return matrix.reduce((maximum: IMaximumResult, row: T[], index: number) => (
    row.length > maximum.value ? {index, value: row.length} : maximum
  ), {index: -1, value: 0});
}

function getLengths<T>(matrix: T[][]): number[] {
  return matrix.map(x => x.length);
}

function equalParity(parity: number, x: number) {
  return (x & 1) === parity;
}

function allLengthsHaveParity<T>(matrix: T[][], parity: number) {
  return matrix.every((list: T[]) => equalParity(parity, list.length));
}

function doesRequireInterlacing(places: IPlaceNode[][], transitions: INode[][]) {
  const parity = places[0].length & 1;
  return !allLengthsHaveParity(places, parity) || !allLengthsHaveParity(transitions, parity);
}

function PlaceNodes(props: IPlaceNodesProps) {
  return (
    <Diagram>
      {
        props.places.map((placeRow: IPlaceNode[], index: number) => (
          <PlaceRow
            nodes={placeRow}
            row={index * 2}
            columns={props.columns}
            interlace={props.interlace}
          />
        ))
      }
    </Diagram>
  );
}

function TransitionNodes(props: ITransitionNodesProps) {
  return (
    <Diagram>
      {
        props.transitions.map((transitionRow: INode[], index: number) => (
          <TransitionRow
            nodes={transitionRow}
            row={(index * 2) + 1}
            columns={props.columns}
            interlace={props.interlace}
          />
        ))
      }
    </Diagram>
  );
}

function Nodes(props: ILatexNodesProps) {
  return (
    <Diagram>
      <TransitionNodes 
        transitions={props.transitions} 
        columns={props.columns}
        interlace={props.interlace}
      />
      <PlaceNodes 
        places={props.places} 
        columns={props.columns} 
        interlace={props.interlace}
      />
    </Diagram>
  );
}

function getEdgeSource(edge: any) {
  if (typeof edge.source === 'number') {
    return edge.source;
  }
  return edge.source.id;
}

function getEdgeTarget(edge: any) {
  if (typeof edge.target === 'number') {
    return edge.target;
  }
  return edge.target.id;
}

function Edges(props: IEdgesProps) {
  return (
    <Diagram>
      {
        props.edges.map((edge: any, index: number) => (
          <Edge
            key={`${props.type}${index}`}
            from={`${getEdgeSource(edge)}`}
            to={`${getEdgeTarget(edge)}`}
          />
        ))
      }
    </Diagram>
  );
}

export function LatexPetriNet({graph} : ILatexPetriNetProps){
  const maximum = findMaximumLength(graph.nodes);
  const interlace = doesRequireInterlacing(graph.nodes, graph.transitions);
  const columns = interlace ? (maximum.value * 2) - 1 : maximum.value;

  return (
    <Diagram>
      <Nodes
        places={graph.nodes} 
        transitions={graph.transitions} 
        columns={columns}
        interlace={interlace}
      />
      <Edges edges={graph.edges.incoming} type={'i'}/>
      <Edges edges={graph.edges.outgoing} type={'i'}/>
    </Diagram>
  );
}
