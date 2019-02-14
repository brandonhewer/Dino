import { Component, MouseEvent } from 'react';
import { Drag } from 'react-dnd';

export type Position = [number, number];

interface IStaticIconProps {
  readonly iconClass: string;
  readonly label: string;
  readonly height: number;
  readonly width: number;
  readonly onMouseDown: (x: number, y: number) => any;
}

interface IActiveIconProps extends IStaticIconProps {
  readonly position: Position;
  readonly onStartDrag: (x: number, y: number) => any;
  readonly onStopDrag: (x: number, y: number) => any;
}

interface IComponentIconProps extends IStaticIconProps {
  readonly onMouseUp?: (x: number, y: number) => any;
}

interface IComponentState {
  dragging: boolean;
  position: Position;
}

export function StaticIcon(props: IStaticIconProps) {
  return (
    <span
      className={props.iconClass}
      role="img"
      aria-label={props.label}
      style={{ width : props.width, height : props.height }}
      onMouseDown={(e: MouseEvent) => props.onMouseDown(e.pageX, e.pageY)}
    />
  );
}

export function DraggableIcon(props: IActiveIconProps): JSX.Element {
  return (
    <Draggable
      axis="both"
      defaultPosition={{ x: props.position[0], y: props.position[1] }}
      onStart={(_: Event, data: DraggableData) => props.onStartDrag(data.x, data.y)}
      onStop={(_: Event, data: DraggableData) => props.onStopDrag(data.x, data.y)}
    >
      <StaticIcon {...props}/>
    </Draggable>
  );
}

export class ComponentIcon extends Component<IComponentIconProps, IComponentState> {
  constructor(props: IComponentIconProps) {
    super(props);
    this.setState({ dragging : false, position : [0, 0] });
  }

  public render(): JSX.Element {
    if (this.state.dragging) {
      return <ActiveIcon position={this.state.position} {...this.props}/>;
    }
    return <StaticIcon onMouseDown={this.onMouseDown} {...this.props}/>;
  }

  private onMouseDown(x: number, y: number) {
    this.setState({ dragging : true, position : [x, y] });
  }

  private onMouseUp(x: number, y: number) {
    this.setState({ dragging : false, position : [x, y] });

    if (this.props.onMouseUp !== undefined) {
      this.props.onMouseUp(x, y);
    }
  }
}
