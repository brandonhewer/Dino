
interface IActionIconProps {
  readonly iconClass: string;
  readonly label: string;
  readonly width: number;
  readonly height: number;
  readonly onClick: (e: MouseEvent) => any;
}

export interface IActionMenuProps {
  readonly iconClasses: string[];
  readonly iconLabels: string[];
  readonly iconWidth: number;
  readonly iconHeight: number;
  readonly onClick: (i: number) => (e: MouseEvent) => any;
}

function ActionIcon(props: IActionIconProps): JSX.Element {
  return (
    <button className="action-icon" onClick={props.onClick}>
      <span
        className={props.iconClass}
        role="img"
        aria-label={props.label}
        style={{ width : props.width, height : props.height }}
      />
    </button>
  );
}

function createIcons(
  iconClasses: string[],
  iconLabels: string[],
  height: number,
  width: number,
  onClick: (i: number) => (e: MouseEvent) => any): JSX.Element[] {
  return iconClasses.map(
    (iconClass, index) => (
      <ActionIcon
        iconClass={iconClass}
        label={iconLabels[index]}
        height={height}
        width={width}
        onClick={onClick(index)}
      />
    ),
  );
}

export function ActionMenu(props: IActionMenuProps): JSX.Element {
  const icons = createIcons(props.iconClasses, props.iconLabels,
                            props.iconHeight, props.iconWidth, props.onClick);
  return (
    <div id="action-menu">
      {icons}
    </div>
  );
}
