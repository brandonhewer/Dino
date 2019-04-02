import { h, Diagram, Node } from 'jsx-tikzcd';
import { IPlaceNode } from '../petri_nets';

interface IPlaceProps {
  readonly key: number;
  readonly position: [number, number];
  readonly hasToken: boolean;
}

interface IPlaceNodeProps {
  readonly key: number;
  readonly position: [number, number];
  readonly variance: number;
  readonly hasToken: boolean;
};

interface IPlaceRowProps {
  readonly nodes: IPlaceNode[];
  readonly row: number;
  readonly columns: number;
  readonly interlace: boolean;
}

const covariantEmptyPlace = '\\tikz[baseline=(char.base)]{\\node[shape=circle,draw,minimum size=15pt,inner sep=0pt,fill=white] (char) {};}';
const contravariantEmptyPlace = '\\tikz[baseline=(char.base)]{\\node[shape=circle,draw,minimum size=15pt,inner sep=0pt,fill=lightgray] (char) {};}';
const covariantPlace = '\\tikz[baseline=(char.base)]{\\node[shape=circle,draw,minimum size=15pt,inner sep=0pt,fill=white] (char) {$f$};}';
const contravariantPlace = '\\tikz[baseline=(char.base)]{\\node[shape=circle,draw,minimum size=15pt,inner sep=0pt,fill=lightgray] (char) {$f$};}';

function CovariantPlace(props: IPlaceProps) {
  return (
    <Node
      key={`${props.key}`}
      position={props.position}
      value={props.hasToken ? covariantPlace : covariantEmptyPlace}
    />
  );
}

function ContravariantPlace(props: IPlaceProps) {
  return (
    <Node
      key={`${props.key}`}
      position={props.position}
      value={props.hasToken ? contravariantPlace : contravariantEmptyPlace}
    />
  );
}

function PlaceNode(props: IPlaceNodeProps) {
  if (props.variance == 0) {
    return (
      <CovariantPlace
        key={props.key}
        position={props.position}
        hasToken={props.hasToken}
      />
    );
  }
  return (
    <ContravariantPlace
      key={props.key}
      position={props.position}
      hasToken={props.hasToken}
    />
  );
}

export function PlaceRow(props: IPlaceRowProps) {
  const rowLength = props.interlace ? (props.nodes.length * 2) - 1 : props.nodes.length;
  const startColumn = Math.ceil((props.columns - rowLength) / 2.0);
  const space = props.interlace ? 2 : 1;

  return (
    <Diagram>
      {
        props.nodes.map((node: IPlaceNode, offset: number) => (
          <PlaceNode 
            key={node.id}
            position={[startColumn + offset * space, props.row]}
            variance={node.variance}
            hasToken={node.count > 0}
          />
        ))
      }
    </Diagram>
  )
}
