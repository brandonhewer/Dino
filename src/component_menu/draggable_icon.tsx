import { Component, MouseEvent } from 'react';

export interface IPosition {
  x: number;
  y: number;
}

interface IDraggableIconProps {
  readonly iconClass: string;
  readonly label: string;
  readonly height: number;
  readonly width: number;
  readonly position: IPosition;
}

function createStyle(position: IPosition, props: IDraggableIconProps) {
  return {
    height : props.height,
    left : `${position.x}px`,
    top : `${position.y}py`,
    width : props.width,
  };
}

export class DraggableIcon extends Component<IDraggableIconProps, IPosition> {
  constructor(props: IDraggableIconProps) {
    super(props);
    this.state = props.position;
  }

  public onMouseMove(e: MouseEvent) {
    this.setState({ x : e.pageX, y : e.pageY });
  }

  public render(): JSX.Element {
    return (
      <span
        className={this.props.iconClass}
        role="img"
        aria-label={this.props.label}
        style={createStyle(this.state, this.props)}
      />
    );
  }
}
