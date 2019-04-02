import { h, Diagram, Node } from 'jsx-tikzcd';
import { INode } from '../petri_nets';

interface ITransitionProps {
  readonly key: number;
  readonly position: [number, number];
};

interface ITransitionRowProps {
  readonly nodes: INode[];
  readonly row: number;
  readonly columns: number;
  readonly interlace: boolean;
};

const transitionNode = '\\tikz[baseline=(char.base)]{\\node[shape=rectangle,draw,minimum size=10pt,inner sep=0pt,fill=black] (char) {};}';

function Transition(props: ITransitionProps) {
  return (
    <Node
      key={`${props.key}`}
      position={props.position}
      value={transitionNode}
    />
  );
}

export function TransitionRow(props: ITransitionRowProps) {
  const rowLength = props.interlace ? (props.nodes.length * 2) - 1 : props.nodes.length;
  const startColumn = Math.ceil((props.columns - rowLength) / 2.0);
  const space = props.interlace ? 2 : 1;

  return (
    <Diagram>
      {
        props.nodes.map((node: INode, offset: number) => (
          <Transition
            key={node.id} 
            position={[startColumn + offset * space, props.row]}
          />
        ))
      }
    </Diagram>
  );
}
