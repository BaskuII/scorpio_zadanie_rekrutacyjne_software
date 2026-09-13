from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import ComposableNodeContainer
from launch_ros.descriptions import ComposableNode
from launch_ros.parameter_descriptions import ParameterValue

PACKAGE = 'scorpio_zadanie_rekrutacyjne_software'
NAMESPACE = 'scorpio'


def argument(name, value_type):
    """Argument launcha przekazany dalej jako parametr węzła, z właściwym typem."""
    return ParameterValue(LaunchConfiguration(name), value_type=value_type)


def generate_launch_description():
    return LaunchDescription([
        DeclareLaunchArgument('input_file', description='Plik z celami (wymagany)'),
        DeclareLaunchArgument('encoder_period', default_value='0.05',
                              description='Okres odczytu enkoderów w sekundach'),
        DeclareLaunchArgument('preempt', default_value='false',
                              description='Wywłaszczanie celu (zadanie 3)'),
        DeclareLaunchArgument('debug', default_value='false',
                              description='Tryb debugowania symulacji'),

        ComposableNodeContainer(
            package='rclcpp_components', executable='component_container_mt',
            name='platform_container', namespace=NAMESPACE, output='screen',
            composable_node_descriptions=[
                ComposableNode(
                    package=PACKAGE, plugin='backend::RosInterface',
                    name='interface', namespace=NAMESPACE,
                    parameters=[{
                        'input_file': argument('input_file', str),
                        'encoder_period': argument('encoder_period', float),
                        'debug': argument('debug', bool),
                    }]),
                ComposableNode(
                    package=PACKAGE, plugin='solution::Solution',
                    name='solution', namespace=NAMESPACE,
                    parameters=[{'preempt': argument('preempt', bool)}]),
            ]),
    ])
