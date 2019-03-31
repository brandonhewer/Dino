import * as d3 from 'd3';
import { Component } from 'react';
import { Edges, IEdge, IEdges } from './edges';
import { INode, IPlaceNode, PlaceNodes } from './place_nodes';
import { TransitionNodes } from './transition_nodes';

export interface IPetriNet {
  readonly nodes: IPlaceNode[][];
  readonly transitions: INode[];
  readonly edges: IEdges;
}

interface IPetriNetProps {
  readonly graphData: IPetriNet;
  readonly width: number;
  readonly height: number;
  readonly liveColour: string;
  readonly deadColour: string;
  readonly placeSize: number;
  readonly transitionSize: number;
  readonly zIndex: number;
  readonly setSVGReference: (x: SVGSVGElement) => void;
}

function all(xs: boolean[]): boolean {
  return xs.reduce((x, y) => x && y, true);
}

function createSimulation(nodes: INode[], edges: IEdges, centre: [number, number]): any {
  const links = edges.incoming.concat(
    edges.outgoing,
    edges.invisible,
  );

  const simulation: any = d3.forceSimulation()
    .force('link', d3.forceLink().distance((d: IEdge) => d.distance * 30))
    .force('charge', d3.forceManyBody().strength(-300))
    .force('center', d3.forceCenter(...centre))
    .nodes(nodes);

  simulation.force('link').links(links);
  return simulation;
}

interface IPetriNetDiagramProps {
  nodes: INode[];
  simulation: any;
}

export class PetriNetDiagram extends Component<IPetriNetProps, IPetriNetDiagramProps> {
  private ref: SVGSVGElement;

  public static getDerivedStateFromProps(nextProps, prevState) {
    const nodes = nextProps.graphData.transitions.concat(...nextProps.graphData.nodes);
    // prevState.simulation.stop();
    return {nodes, simulation: createSimulation(nodes, nextProps.graphData.edges, [
      nextProps.width / 2, nextProps.height / 2,
    ])};
  }

  constructor(props: IPetriNetProps) {
    super(props);

    const nodes = props.graphData.transitions.concat(...props.graphData.nodes);

    this.state = {
      nodes,
      simulation: createSimulation(nodes, props.graphData.edges, [
        this.props.width / 2, this.props.height / 2,
      ]),
    };
  }

  public componentDidMount(): void {
    this.props.setSVGReference(this.ref);
    this.runSimulation(this.state.simulation);
  }

  public componentDidUpdate(): void {
    this.props.setSVGReference(this.ref);
    this.runSimulation(this.state.simulation);
  }

  public render(): JSX.Element {
    return (
      <svg 
        ref={(r) => this.ref = r} 
        className="petrinet" 
        height={this.props.height} 
        width={this.props.width}
        z-index={this.props.zIndex}
      >
        <defs>
          <marker
            id="arrow"
            viewBox="0 0 10 10"
            refX="5"
            refY="5"
            markerWidth="6"
            markerHeight="6"
            orient="auto-start-reverse"
          >
            <path d="M 0 0 L 10 5 L 0 10 z" />
          </marker>
        </defs>
        <Edges
          incoming={this.props.graphData.edges.incoming}
          outgoing={this.props.graphData.edges.outgoing}
          invisible={this.props.graphData.edges.invisible}
        />
        <PlaceNodes
          nodes={this.props.graphData.nodes}
          radius={this.props.placeSize}
          simulation={this.state.simulation}
        />
        <TransitionNodes
          transitions={this.props.graphData.transitions}
          width={this.props.transitionSize}
          height={this.props.transitionSize}
          simulation={this.state.simulation}
          fillColour={this.getTransitionColour.bind(this)}
          onClick={this.fireIfActive.bind(this)}
        />
      </svg>
    );
  }

  private fire(incoming: INode[], outgoing: INode[]) {
    incoming.forEach((node: any) => --node.count);
    outgoing.forEach((node: any) => ++node.count);
    this.setState({ nodes: incoming.concat(outgoing) });
  }

  private fireIfActive(node: INode, _: any): void {
    const edges = this.props.graphData.edges;
    const incoming = edges.incoming.filter((edge: any) => (
      edge.target.id === node.id
    ));

    if (all(incoming.map((edge: any) => edge.source.count > 0))) {
      const outgoing = edges.outgoing.filter((edge: any) => (
        edge.source.id === node.id
      ));

      this.fire(
        incoming.map((edge: any) => edge.source),
        outgoing.map((edge: any) => edge.target),
      );
    }
  }

  private isActive(node: INode): boolean {
    const incoming = this.props.graphData.edges.incoming.filter((edge: any) => (
      edge.target.id === node.id
    ));
    return all(incoming.map((edge: any) => edge.source.count > 0));
  }

  private getTransitionColour(node: INode): string {
    return this.isActive(node) ? this.props.liveColour : this.props.deadColour;
  }

  private runSimulation(simulation: any): void {
    const context = d3.select(this.ref);

    const nodes = context.select('.places').selectAll('.nodes');
    const place = nodes.selectAll('circle.node');
    const text = nodes.selectAll('text');

    const transition = context.select('.transitions').selectAll('rect.node');
    const offset = this.props.transitionSize / 2;

    const edges = context.select('.edges');
    const edge = edges.selectAll('polyline.link');
    const invisibleEdge = edges.selectAll('line.link');

    simulation.nodes(this.state.nodes).on('tick', () => {
      edge.attr('points', (d: any) => {
        const sx = d.source.x;
        const sy = d.source.y;
        const ex = d.target.x;
        const ey = d.target.y;
        const mx = (sx + ex) / 2;
        const my = (sy + ey) / 2;
        return `${sx},${sy} ${mx},${my} ${ex},${ey}`;
      });
      invisibleEdge
        .attr('x1', (d: any) => d.source.x)
        .attr('y1', (d: any) => d.source.x)
        .attr('x2', (d: any) => d.target.x)
        .attr('y2', (d: any) => d.target.x);
      place
        .attr('cx', (d: any) => d.x)
        .attr('cy', (d: any) => d.y);
      text
        .attr('x', (d: any) => d.x - 5)
        .attr('y', (d: any) => d.y + 5);
      transition
        .attr('x', (d: any) => d.x - offset)
        .attr('y', (d: any) => d.y - offset);
    });

    context.call(d3.zoom().on("zoom", () => {
      nodes.attr("transform", d3.event.transform);
      transition.attr("transform", d3.event.transform);
      edge.attr("transform", d3.event.transform);
      invisibleEdge.attr("transform", d3.event.transform);
      text.attr("font-size", (12 / d3.event.scale) + "px");
    }));
  }
}
