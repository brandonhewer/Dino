import { DraggableIcon, StaticIcon } from './component_icon';

interface IComponentRowProps {
  readonly icons: JSX.Element[];
}

export class ComponentRow {

}

function ComponentRow(props: IComponentRowProps): JSX.Element {
  return (
    <div className="component-row">
      {props.icons}
    </div>
  );
}

export interface IComponentMenuSetup {
  readonly iconClasses: string[];
  readonly iconLabels: string[];
  readonly iconWidth: number;
  readonly iconHeight: number;
  readonly iconsInRow: number;
}

export interface IComponentMenuProps extends IComponentMenuSetup {
  readonly onReleaseComponent: (i: number) => any;
}

function range(N: number): number[] {
  return Array.from(Array(N).keys());
}

function subslices<T>(list: T[], cutEvery: number): T[][] {
  const getSlice = (i: number) => list.slice(i * cutEvery, (i + 1) * cutEvery);
  return range(Math.ceil(list.length / cutEvery)).map(getSlice);
}

function makeRow(icons: JSX.Element[]): JSX.Element {
  return <ComponentRow icons={icons}/>;
}

function makeRows(icons: JSX.Element[], iconsInRow: number): JSX.Element[] {
  return subslices(icons, iconsInRow).map(makeRow);
}

function createIcons(
  iconClasses: string[],
  iconLabels: string[],
  height: number,
  width: number,
  onMouseUp: (i: number) => (e: MouseEvent) => any): JSX.Element[] {
  return iconClasses.map(
    (iconClass, index) => (
      <ComponentIcon
        iconClass={iconClass}
        label={iconLabels[index]}
        height={height}
        width={width}
        onMouseUp={onMouseUp(index)}
      />
    ),
  );
}

export function ComponentMenu(props: IComponentMenuProps): JSX.Element {
  const onRelease = (i: number) => (e: MouseEvent) => props.onReleaseComponent(i);
  const icons = createIcons(props.iconClasses, props.iconLabels,
                            props.iconHeight, props.iconWidth, onRelease);
  return (
    <div id="component-menu">
      {makeRows(icons, props.iconsInRow)}
    </div>
  );
}
