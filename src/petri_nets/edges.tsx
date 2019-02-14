import * as d3 from 'd3';
import { Component } from 'react';

export interface IEdge {
  readonly source: number;
  readonly target: number;
  readonly distance: number;
}

export interface IEdges {
  readonly incoming: IEdge[];
  readonly outgoing: IEdge[];
  readonly invisible: IEdge[];
}

export class Edges extends Component<IEdges, {}> {
  private ref: SVGGElement;

  public componentDidUpdate(): void {
    this.drawEdges();
  }

  public componentDidMount(): void {
    this.drawEdges();
  }

  public render(): JSX.Element {
    return <g className="edges" ref={(ref: SVGGElement) => this.ref = ref}/>;
  }

  private drawEdges() {
    const edges = d3.select(this.ref)
      .selectAll('polyline.link')
      .data(this.props.incoming.concat(this.props.outgoing));

    edges.enter().append('polyline')
      .attr('class', 'link')
      .attr('marker-mid', 'url(#arrow)')
      .style('stroke', '#000')
      .style('stroke-width', 2);

    edges.exit().remove();

    const invisible = d3.select(this.ref)
      .selectAll('line.link')
      .data(this.props.invisible);

    invisible.data(this.props.invisible)
      .enter().append('line')
        .attr('class', 'link')
        .style('stroke-width', 0);

    invisible.exit().remove();
  }
}
